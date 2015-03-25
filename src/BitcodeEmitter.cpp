/*
 * Monicelli: an esoteric language compiler
 * 
 * Copyright (C) 2014 Stefano Sanfilippo
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "BitcodeEmitter.hpp"
#include "Scope.hpp"
#include "Nodes.hpp"
#include "ModuleRegistry.hpp"

#include <llvm/IR/Verifier.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Analysis/Passes.h>

#include <cassert>
#include <string>
#include <map>
#include <vector>
#include <unordered_set>
#include <initializer_list>
#include <unordered_map>
#include <iostream>

// Yes, that's right, no ending ;
#define GUARDED(call) if (!(call)) return false


using namespace monicelli;
using llvm::getGlobalContext;


struct BitcodeEmitter::Private {
    llvm::Value *retval = nullptr;
    llvm::AllocaInst *funcRetval = nullptr;
    llvm::BasicBlock *funcExit = nullptr;

    llvm::IRBuilder<> builder = llvm::IRBuilder<>(getGlobalContext());
    Scope<std::string, llvm::AllocaInst*> scope;
    Pointer<llvm::legacy::FunctionPassManager> optimizer;
};

static
llvm::AllocaInst* allocateVar(llvm::Function *func, Id const& name, llvm::Type *type) {
    llvm::IRBuilder<> builder(&func->getEntryBlock(), func->getEntryBlock().begin());
    return builder.CreateAlloca(type, 0, name.getValue().c_str());
}

static
llvm::AllocaInst* allocateReturnVariable(llvm::Function *func) {
    llvm::IRBuilder<> builder(&func->getEntryBlock(), func->getEntryBlock().begin());
    return builder.CreateAlloca(func->getReturnType(), 0, "result");
}

static
bool reportError(std::initializer_list<std::string> const& what) {
    for (std::string const& chunk: what) {
        std::cerr << chunk << ' ';
    }
    std::cerr << std::endl;

    return false;
}

#define I64 llvm::Type::getInt64Ty(getGlobalContext())
#define  I8 llvm::Type::getInt8Ty(getGlobalContext())
#define  I1 llvm::Type::getInt1Ty(getGlobalContext())
#define   F llvm::Type::getFloatTy(getGlobalContext())
#define   D llvm::Type::getDoubleTy(getGlobalContext())
#define   V llvm::Type::getVoidTy(getGlobalContext())

static const std::unordered_map<llvm::Type*, std::unordered_map<llvm::Type*, llvm::Type*>> TYPECAST_MAP = {
    {I64, {            {I8, I64}, {I1, I64}, { F, D}, {D, D}}},
    { I8, {{I64, I64},            {I1,  I8}, { F, F}, {D, D}}},
    { I1, {{I64, I64}, {I8,  I8},            { F, F}, {D, D}}},
    {  F, {{I64,   D}, {I8,   F}, {I1,   F},          {D, D}}},
    {  D, {{I64,   D}, {I8,   D}, {I1,   D}, { F, D}        }}
};

static
Type MonicelliType(llvm::Type const* type) {
    if (type == I64) {
        return Type::INT;
    } else if (type == I8) {
        return Type::CHAR;
    } else if (type == I1) {
        return Type::BOOL;
    } else if (type == D) {
        return Type::DOUBLE;
    } else if (type == F) {
        return Type::FLOAT;
    } else if (type == V) {
        return Type::VOID;
    }

    return Type::UNKNOWN;
}

static
llvm::Type *LLVMType(Type const& type) {
    switch (type) {
        case Type::INT:
            return I64;
        case Type::CHAR:
            return I8;
        case Type::FLOAT:
            return F;
        case Type::BOOL:
            return I1;
        case Type::DOUBLE:
            return D;
        case Type::VOID:
            return V;
        case Type::UNKNOWN:
            return nullptr; // FIXME
    }

    return nullptr;
}

static
llvm::Type* deduceResultType(llvm::Value *left, llvm::Value *right) {
    llvm::Type *lt = left->getType();
    llvm::Type *rt = right->getType();

    if (lt == rt) return rt;

    auto subTable = TYPECAST_MAP.find(lt);
    if (subTable != TYPECAST_MAP.end()) {
        auto resultType = subTable->second.find(rt);
        if (resultType != subTable->second.end()) return resultType->second;
    }

    return nullptr;
}

#undef I64
#undef  I8
#undef  I1
#undef   F
#undef   D
#undef   V

static inline
bool isFP(llvm::Type *type) {
    return type->isFloatTy() || type->isDoubleTy();
}

static inline
bool isInt(llvm::Type *type) {
    return type->isIntegerTy();
}

static
llvm::Value* coerce(BitcodeEmitter::Private *d, llvm::Value *val, llvm::Type *toType) {
    llvm::Type *fromType = val->getType();

    if (fromType == toType) return val;

    if (isInt(toType)) {
        if (isFP(fromType)) {
            return d->builder.CreateFPToSI(val, toType);
        } else if (isInt(fromType)) {
            return d->builder.CreateSExtOrTrunc(val, toType);
        }
    }
    else if (isFP(toType) && isInt(fromType)) {
        return d->builder.CreateSIToFP(val, toType);
    }
    else if (fromType->isFloatTy() && toType->isDoubleTy()) {
        return d->builder.CreateFPExt(val, toType);
    }
    else if (fromType->isDoubleTy() && toType->isFloatTy()) {
        return d->builder.CreateFPTrunc(val, toType);
    }

    return nullptr;
}

static
llvm::Value* isTrue(BitcodeEmitter::Private *d, llvm::Value* test, llvm::Twine const& label="") {
    llvm::Value *one = llvm::ConstantInt::get(getGlobalContext(), llvm::APInt(1, 0));
    return d->builder.CreateICmpNE(
        coerce(d, test, one->getType()), one, label
    );
}

static const std::map<Type, std::string> PUT_NAMES = {{
    {Type::BOOL, "__Monicelli_putBool"},
    {Type::CHAR, "__Monicelli_putChar"},
    {Type::FLOAT, "__Monicelli_putFloat"},
    {Type::DOUBLE, "__Monicelli_putDouble"},
    {Type::INT, "__Monicelli_putInt"}
}};

static const std::map<Type, std::string> GET_NAMES = {
    {Type::BOOL, "__Monicelli_getBool"},
    {Type::CHAR, "__Monicelli_getChar"},
    {Type::FLOAT, "__Monicelli_getFloat"},
    {Type::DOUBLE, "__Monicelli_getDouble"},
    {Type::INT, "__Monicelli_getInt"}
};

static const std::string ABORT_NAME = "__Monicelli_abort";
static const std::string ASSERT_NAME = "__Monicelli_assert";

static
bool convertAndStore(BitcodeEmitter::Private *d, llvm::AllocaInst *dest, llvm::Value *expression) {
    llvm::Type *varType = dest->getAllocatedType();
    expression = coerce(d, expression, varType);
    if (expression == nullptr) return false;
    d->builder.CreateStore(expression, dest);
    return true;
}

BitcodeEmitter::BitcodeEmitter() {
    module = std::unique_ptr<llvm::Module>(
        new llvm::Module("monicelli", getGlobalContext())
    );
    d = new Private;

    d->optimizer = Pointer<llvm::legacy::FunctionPassManager>(
        new llvm::legacy::FunctionPassManager(module.get())
    );

    d->optimizer->add(llvm::createBasicAliasAnalysisPass());
    d->optimizer->add(llvm::createInstructionCombiningPass());
    d->optimizer->add(llvm::createReassociatePass());
    d->optimizer->add(llvm::createGVNPass());
    d->optimizer->add(llvm::createCFGSimplificationPass());
    d->optimizer->doInitialization();
}

BitcodeEmitter::~BitcodeEmitter() {
    delete d;
}

bool BitcodeEmitter::emit(Return const& node) {
    if (node.getExpression()) {
        GUARDED(node.getExpression()->emit(this));
        llvm::Type *type = d->builder.GetInsertBlock()->getParent()->getReturnType();
        assert(d->funcRetval != nullptr);
        d->builder.CreateStore(coerce(d, d->retval, type), d->funcRetval);
    }

    d->builder.CreateBr(d->funcExit);

    return true;
}

bool BitcodeEmitter::emit(Loop const& node) {
    llvm::Function *father = d->builder.GetInsertBlock()->getParent();

    llvm::BasicBlock *body = llvm::BasicBlock::Create(
        getGlobalContext(), "loop", father
    );

    d->builder.CreateBr(body);
    d->builder.SetInsertPoint(body);

    llvm::BasicBlock *condition = llvm::BasicBlock::Create(
        getGlobalContext(), "loopcondition"
    );

    GUARDED(ensureBasicBlock(node.getBody(), condition));

    father->getBasicBlockList().push_back(condition);
    d->builder.SetInsertPoint(condition);

    GUARDED(node.getCondition().emit(this));

    llvm::Value *loopTest = isTrue(d, d->retval, "looptest");

    llvm::BasicBlock *after = llvm::BasicBlock::Create(
        getGlobalContext(), "afterloop", father
    );

    d->builder.CreateCondBr(loopTest, body, after);
    d->builder.SetInsertPoint(after);

    return true;
}

bool BitcodeEmitter::emit(VarDeclaration const& node) {
    llvm::Function *father = d->builder.GetInsertBlock()->getParent();
    llvm::Type *varType = LLVMType(node.getType());
    llvm::AllocaInst *alloc = allocateVar(father, node.getId(), varType);

    if (node.getInitializer()) {
        GUARDED(node.getInitializer()->emit(this));
        if (!convertAndStore(d, alloc, d->retval)) {
            return reportError({
                "Invalid inizializer for variable", node.getId().getValue()
            });
        }
    }

    // TODO pointers

    d->scope.push(node.getId().getValue(), alloc);

    return true;
}

bool BitcodeEmitter::emit(Assignment const& node) {
    auto var = d->scope.lookup(node.getName().getValue());

    if (!var) {
        return reportError({
            "Attempting assignment to undefined var", node.getName().getValue()
        });
    }

    GUARDED(node.getValue().emit(this));
    if (!convertAndStore(d, *var, d->retval)) {
        return reportError({
            "Invalid assignment to variable", node.getName().getValue()
        });
    }

    return true;
}

bool BitcodeEmitter::emit(Print const& node) {
    std::vector<llvm::Value*> callargs;
    GUARDED(node.getExpression().emit(this));
    callargs.push_back(d->retval);

    Type printType = MonicelliType(d->retval->getType());

    if (printType == Type::UNKNOWN) {
        return reportError({"Attempting to print unknown type"});
    }

    auto toCall = PUT_NAMES.find(printType);

    if (toCall == PUT_NAMES.end()) {
        return reportError({"Unknown print function for type"});
    }

    llvm::Function *callee = module->getFunction(toCall->second);

    if (callee == nullptr) {
        return reportError({"Print function was not registered"});
    }

    d->builder.CreateCall(callee, callargs);

    return true;
}

bool BitcodeEmitter::emit(Input const& node) {
    auto lookupResult = d->scope.lookup(node.getVariable().getValue());

    if (!lookupResult) {
        return reportError({"Attempting to read undefined variable"});
    }

    llvm::AllocaInst *variable = *lookupResult;
    Type inputType = MonicelliType(variable->getAllocatedType());

    if (inputType == Type::UNKNOWN) {
        return reportError({"Attempting to read unknown type"});
    }

    auto toCall = GET_NAMES.find(inputType);

    if (toCall == GET_NAMES.end()) {
        return reportError({
            "Unknown input function for type"
        });
    }

    llvm::Function *callee = module->getFunction(toCall->second);

    if (callee == nullptr) {
        return reportError({
            "Input function was not registered for type"
        });
    }

    llvm::Value *readval = d->builder.CreateCall(callee);
    d->builder.CreateStore(readval, variable);

    return true;
}

bool BitcodeEmitter::emit(Abort const&) {
    llvm::Function *callee = module->getFunction(ABORT_NAME);

    if (callee == nullptr) {
        return reportError({"Abort function was not registered"});
    }

    d->builder.CreateCall(callee);

    return true;
}

bool BitcodeEmitter::emit(Assert const& node) {
    llvm::Function *callee = module->getFunction(ASSERT_NAME);

    if (callee == nullptr) {
        return reportError({"Assert function was not registered"});
    }

    node.getExpression().emit(this);
    d->builder.CreateCall(callee, {coerce(d, d->retval, LLVMType(Type::BOOL))});

    return true;
}

bool BitcodeEmitter::emit(FunctionCall const& node) {
    llvm::Function *callee = module->getFunction(node.getName().getValue());

    if (callee == 0) {
        return reportError({
            "Attempting to call undefined function",
            node.getName().getValue() + "()"
        });
    }

    if (callee->arg_size() != node.getArgs().size()) {
        return reportError({
            "Argument number mismatch in call of",
            node.getName().getValue() + "()",
            "expected", std::to_string(callee->arg_size()),
            "given", std::to_string(node.getArgs().size())
        });
    }

    auto param = callee->getArgumentList().begin();
    std::vector<llvm::Value*> callargs;
    for (Expression const& arg: node.getArgs()) {
        GUARDED(arg.emit(this));
        callargs.push_back(coerce(d, d->retval, param->getType()));
        ++param;
    }

    d->retval = d->builder.CreateCall(callee, callargs);

    return true;
}

bool BitcodeEmitter::emit(Branch const& node) {
    Branch::Body const& body = node.getBody();
    llvm::Function *func = d->builder.GetInsertBlock()->getParent();

    llvm::BasicBlock *thenbb = llvm::BasicBlock::Create(
        getGlobalContext(), "then", func
    );
    llvm::BasicBlock *elsebb = llvm::BasicBlock::Create(
        getGlobalContext(), "else"
    );
    llvm::BasicBlock *mergebb = llvm::BasicBlock::Create(
        getGlobalContext(), "endif"
    );

    assert(!body.getCases().empty());
    BranchCase const& last = body.getCases().back();

    for (BranchCase const& cas: body.getCases()) {
        emitSemiExpression(node.getVar(), cas.getCondition());
        d->builder.CreateCondBr(
            isTrue(d, d->retval, "condition"), thenbb, elsebb
        );
        d->builder.SetInsertPoint(thenbb);

        GUARDED(ensureBasicBlock(cas.getBody(), mergebb));

        func->getBasicBlockList().push_back(elsebb);
        d->builder.SetInsertPoint(elsebb);

        if (&cas != &last) {
            thenbb = llvm::BasicBlock::Create(getGlobalContext(), "then", func);
            elsebb = llvm::BasicBlock::Create(getGlobalContext(), "else");
        }
    }

    if (body.getElse()) {
        GUARDED(ensureBasicBlock(*body.getElse(), mergebb));
    }

    func->getBasicBlockList().push_back(mergebb);
    d->builder.SetInsertPoint(mergebb);

    return true;
}

bool BitcodeEmitter::emit(FunctionPrototype const& node) {
    std::vector<llvm::Type*> argTypes;

    for (FunArg const& arg: node.getArgs()) {
        argTypes.emplace_back(LLVMType(arg.getType()));
    }

    std::unordered_set<std::string> argsSet;
    for (FunArg const& arg: node.getArgs()) {
        std::string const& name = arg.getName().getValue();
        if (argsSet.find(name) != argsSet.end()) {
            return reportError({
                "Two arguments with same name to function",
                node.getName().getValue() + "():", name
            });
        }
        argsSet.insert(name);
    }

    llvm::FunctionType *ftype = llvm::FunctionType::get(
        LLVMType(node.getType()), argTypes, false
    );

    llvm::Function *func = llvm::Function::Create(
        ftype, llvm::Function::ExternalLinkage, node.getName().getValue(), module.get()
    );

    if (func->getName() != node.getName().getValue()) {
        func->eraseFromParent();
        func = module->getFunction(node.getName().getValue());

        if (!func->empty()) {
            return reportError({
                "Redefining function", node.getName().getValue()
            });
        }

        if (func->arg_size() != node.getArgs().size()) {
            return reportError({
                "Argument number mismatch in definition vs declaration of",
                node.getName().getValue() + "()",
                "expected", std::to_string(func->arg_size()),
                "given", std::to_string(node.getArgs().size())
            });
        }
    }

    auto argToEmit = func->arg_begin();
    for (FunArg const& arg: node.getArgs()) {
        argToEmit->setName(arg.getName().getValue());
        ++argToEmit;
    }

    d->retval = func;

    return true;
}

bool BitcodeEmitter::emit(Function const& node) {
    GUARDED(node.getPrototype().emit(this));
    llvm::Function *func = dynamic_cast<llvm::Function*>(d->retval);

    assert(func != nullptr);

    llvm::BasicBlock *bb = llvm::BasicBlock::Create(
        getGlobalContext(), "entry", func
    );
    d->builder.SetInsertPoint(bb);

    bool isNotVoid = node.getPrototype().getType() != Type::VOID;

    d->funcRetval = isNotVoid? allocateReturnVariable(func): nullptr;
    d->funcExit = llvm::BasicBlock::Create(getGlobalContext(), "return");

    d->scope.enter();

    auto argToAlloc = func->arg_begin();
    for (FunArg const& arg: node.getPrototype().getArgs()) {
        llvm::AllocaInst *alloc = allocateVar(
            func, arg.getName(), LLVMType(arg.getType())
        );
        d->builder.CreateStore(argToAlloc, alloc);
        d->scope.push(arg.getName().getValue(), alloc);
        ++argToAlloc;
    }

    for (Statement const& stat: node.getBody()) {
        GUARDED(stat.emit(this));
    }

    d->scope.leave();

    if (!d->builder.GetInsertBlock()->getTerminator()) {
        d->builder.CreateBr(d->funcExit);
    }

    func->getBasicBlockList().push_back(d->funcExit);
    d->builder.SetInsertPoint(d->funcExit);

    if (isNotVoid) {
        d->builder.CreateRet(d->builder.CreateLoad(d->funcRetval));
    } else {
        d->builder.CreateRetVoid();
    }

    verifyFunction(*func);

    d->optimizer->run(*func);

    return true;
}

bool BitcodeEmitter::emit(Module const& node) {
    return true;
}

bool BitcodeEmitter::emit(Program const& program) {
    auto const& externals = getModuleRegistry().getRegisteredFunctions();
    for (FunctionPrototype const& proto: externals) {
        GUARDED(proto.emit(this));
    }

    for (Function const& function: program.getFunctions()) {
        GUARDED(function.getPrototype().emit(this));
    }

    for (Function const& function: program.getFunctions()) {
        GUARDED(function.emit(this));
    }

    if (program.getMain()) {
        GUARDED(program.getMain()->emit(this));
    }

    verifyModule(*module);

    return true;
}

bool BitcodeEmitter::emit(Id const& node) {
    auto value = d->scope.lookup(node.getValue());

    if (!value) {
        return reportError({
            "Undefined variable", node.getValue()
        });
    }

    d->retval = d->builder.CreateLoad(*value, node.getValue().c_str());

    return true;
}

bool BitcodeEmitter::emit(Integer const& node) {
    d->retval = llvm::ConstantInt::get(
        getGlobalContext(), llvm::APInt(64, node.getValue(), true)
    );

    return true;
}

bool BitcodeEmitter::emit(Float const& node) {
    d->retval = llvm::ConstantFP::get(
        getGlobalContext(), llvm::APFloat(node.getValue())
    );

    return true;
}

#define HANDLE(intop, fpop) \
    if (fp) { \
        d->retval = d->builder.Create##fpop(left, right); \
    } else { \
        d->retval = d->builder.Create##intop(left, right); \
    }

#define HANDLE_INT_ONLY(op, symbol) \
    if (fp) { \
        return reportError({"Operator " #symbol " cannot be applied to float values!"}); \
    } else { \
        d->retval = d->builder.Create##op(left, right); \
    }

static
bool createOp(BitcodeEmitter::Private *d, llvm::Value *left, Operator op, llvm::Value *right) {
    llvm::Type *retType = deduceResultType(left, right);

    if (retType == nullptr) {
        return reportError({"Cannot combine operators."});
    }

    bool fp = isFP(retType);

    left = coerce(d, left, retType);
    right = coerce(d, right, retType);

    if (left == nullptr || right == nullptr) {
        return reportError({"Cannot convert operators to result type."});
    }

    switch (op) {
        case Operator::PLUS:
            HANDLE(Add, FAdd)
            break;
        case Operator::MINUS:
            HANDLE(Sub, FSub)
            break;
        case Operator::TIMES:
            HANDLE(Mul, FMul)
            break;
        case Operator::DIV:
            HANDLE(SDiv, FDiv)
            break;
        case Operator::SHL:
            HANDLE_INT_ONLY(Shl, <<);
            break;
        case Operator::SHR:
            HANDLE_INT_ONLY(LShr, >>);
            break;
        case Operator::LT:
            HANDLE(ICmpULT, FCmpULT)
            break;
        case Operator::GT:
            HANDLE(ICmpUGT, FCmpUGT)
            break;
        case Operator::GTE:
            HANDLE(ICmpUGE, FCmpUGE)
            break;
        case Operator::LTE:
            HANDLE(ICmpULE, FCmpULE)
            break;
        case Operator::EQ:
            HANDLE(ICmpEQ, FCmpOEQ)
            break;
    }

    return true;
}

#undef HANDLE
#undef HANDLE_INT_ONLY

bool BitcodeEmitter::emit(BinaryExpression const& expression) {
    GUARDED(expression.getLeft().emit(this));
    llvm::Value *left = d->retval;

    GUARDED(expression.getRight().emit(this));
    llvm::Value *right = d->retval;

    GUARDED(createOp(d, left, expression.getOperator(), right));

    return true;
}

bool BitcodeEmitter::emitSemiExpression(Id const& left, SemiExpression const& right) {
    GUARDED(left.emit(this));
    llvm::Value *lhs = d->retval;

    GUARDED(right.getLeft().emit(this));
    llvm::Value *rhs = d->retval;

    GUARDED(createOp(d, lhs, right.getOperator(), rhs));

    return true;
}

bool BitcodeEmitter::ensureBasicBlock(PointerList<Statement> const& statements, llvm::BasicBlock *after) {
    d->scope.enter();
    for (Statement const& statement: statements) {
        GUARDED(statement.emit(this));
    }
    d->scope.leave();

    if (!d->builder.GetInsertBlock()->getTerminator()) {
        d->builder.CreateBr(after);
    }

    return true;
}

