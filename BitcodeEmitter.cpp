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

#include <llvm/Analysis/Verifier.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include <string>
#include <vector>


using namespace monicelli;
using llvm::getGlobalContext;


struct BitcodeEmitter::Private {
    llvm::Value *retval = nullptr;

    llvm::IRBuilder<> builder = llvm::IRBuilder<>(getGlobalContext());
    Scope<std::string, llvm::AllocaInst*> scope;
};

static
llvm::Type *LLVMType(Type const& type) {
    switch (type) {
        case Type::INT:
            return llvm::Type::getInt64Ty(getGlobalContext());
        case Type::CHAR:
            return llvm::Type::getInt8Ty(getGlobalContext());
        case Type::FLOAT:
            return llvm::Type::getFloatTy(getGlobalContext());
        case Type::BOOL:
            return llvm::Type::getInt1Ty(getGlobalContext());
        case Type::DOUBLE:
            return llvm::Type::getDoubleTy(getGlobalContext());
        case Type::VOID:
            return llvm::Type::getVoidTy(getGlobalContext());
    }
}

static
llvm::AllocaInst* allocateVar(llvm::Function *func, Id const& name, llvm::Type *type) {
    llvm::IRBuilder<> builder(&func->getEntryBlock(), func->getEntryBlock().begin());
    return builder.CreateAlloca(type, 0, name.getValue().c_str());
}

static
llvm::Value* isTrue(llvm::IRBuilder<> &builder, llvm::Value* test, llvm::Twine const& label="") {
    return builder.CreateICmpNE(
        test,
        llvm::ConstantInt::get(getGlobalContext(), llvm::APInt(1, 0)),
        label
    );
}

BitcodeEmitter::BitcodeEmitter() {
    module = std::unique_ptr<llvm::Module>(new llvm::Module("monicelli", getGlobalContext()));
    d = new Private;
}

BitcodeEmitter::~BitcodeEmitter() {
    delete d;
}

void BitcodeEmitter::emit(Return const& node) {
    if (node.getExpression()) {
        node.getExpression()->emit(this);
        d->builder.CreateRet(d->retval);
    } else {
        d->builder.CreateRetVoid();
    }
}

void BitcodeEmitter::emit(Loop const& node) {
    llvm::Function *father = d->builder.GetInsertBlock()->getParent();

    llvm::BasicBlock *body = llvm::BasicBlock::Create(
        getGlobalContext(), "loop", father
    );

    d->builder.CreateBr(body);
    d->builder.SetInsertPoint(body);

    d->scope.enter();
    for (Statement const* statement: node.getBody()) {
        statement->emit(this);
    }
    d->scope.leave();

    node.getCondition().emit(this);

    llvm::Value *loopTest = isTrue(d->builder, d->retval, "looptest");

    llvm::BasicBlock *after = llvm::BasicBlock::Create(
        getGlobalContext(), "afterloop", father
    );

    d->builder.CreateCondBr(loopTest, body, after);
    d->builder.SetInsertPoint(after);
}

void BitcodeEmitter::emit(VarDeclaration const& node) {
    llvm::Function *father = d->builder.GetInsertBlock()->getParent();
    llvm::AllocaInst *alloc = allocateVar(
        father, node.getId(), LLVMType(node.getType())
    );

    if (node.getInitializer()) {
        node.getInitializer()->emit(this);
        d->builder.CreateStore(d->retval, alloc);
    }

    // TODO pointers

    d->scope.push(node.getId().getValue(), alloc);
}

void BitcodeEmitter::emit(Assignment const& node) {
    node.getValue().emit(this);
    auto var = d->scope.lookup(node.getName().getValue());

    if (!var) {
        // TODO undefined var
    }

    d->builder.CreateStore(d->retval, *var);
}

void BitcodeEmitter::emit(Print const& node) {
}

void BitcodeEmitter::emit(Input const& node) {
}

void BitcodeEmitter::emit(Abort const& node) {
}

void BitcodeEmitter::emit(Assert const& node) {
}

void BitcodeEmitter::emit(FunctionCall const& node) {
    llvm::Function *callee = module->getFunction(node.getName().getValue());

    if (callee == 0) {
        // TODO Error
    }

    if (callee->arg_size() != node.getArgs().size()) {
        // TODO Error
    }

    std::vector<llvm::Value*> callargs;
    for (Expression const* arg: node.getArgs()) {
        arg->emit(this);
        callargs.push_back(d->retval);
    }

    d->retval = d->builder.CreateCall(callee, callargs);
}

void BitcodeEmitter::emit(Branch const& node) {
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

    BranchCase const* last = body.getCases().back();

    for (BranchCase const* cas: body.getCases()) {
        emitSemiExpression(node.getVar(), cas->getCondition());
        d->builder.CreateCondBr(
            isTrue(d->builder, d->retval, "condition"), thenbb, elsebb
        );
        d->builder.SetInsertPoint(thenbb);
        d->scope.enter();
        for (Statement const* statement: cas->getBody()) {
            statement->emit(this);
        }
        d->scope.leave();
        d->builder.CreateBr(mergebb);

        func->getBasicBlockList().push_back(elsebb);
        d->builder.SetInsertPoint(elsebb);

        if (cas != last) {
            thenbb = llvm::BasicBlock::Create(getGlobalContext(), "then", func);
            elsebb = llvm::BasicBlock::Create(getGlobalContext(), "else");
        }
    }

    if (body.getElse()) {
        d->scope.enter();
        for (Statement const* statement: *body.getElse()) {
            statement->emit(this);
        }
        d->scope.leave();
        d->builder.CreateBr(mergebb);
    }

    func->getBasicBlockList().push_back(mergebb);
    d->builder.SetInsertPoint(mergebb);
}

void BitcodeEmitter::emit(Function const& node) {
    d->scope.enter();

    std::vector<llvm::Type*> argtypes;

    for (FunArg const* arg: node.getArgs()) {
        argtypes.emplace_back(LLVMType(arg->getType()));
    }

    llvm::FunctionType *ftype = llvm::FunctionType::get(
        LLVMType(node.getType()), argtypes, false
    );

    llvm::Function *func = llvm::Function::Create(
        ftype, llvm::Function::ExternalLinkage, node.getName().getValue(), module.get()
    );

    if (func->getName() != node.getName().getValue()) {
        func->eraseFromParent();
        func = module->getFunction(node.getName().getValue());

        if (!func->empty()) {
            // TODO redefinition
        }

        if (func->arg_size() != node.getArgs().size()) {
            // TODO different args
        }
    }

    auto argToEmit = func->arg_begin();
    for (FunArg const* arg: node.getArgs()) {
        argToEmit->setName(arg->getName().getValue());
        ++argToEmit;
    }

    // TODO check repeated arg names

    llvm::BasicBlock *bb = llvm::BasicBlock::Create(
        getGlobalContext(), "entry", func
    );
    d->builder.SetInsertPoint(bb);

    auto argToAlloc = func->arg_begin();
    for (FunArg const* arg: node.getArgs()) {
        llvm::AllocaInst *alloc = allocateVar(
            func, arg->getName(), LLVMType(arg->getType())
        );
        d->builder.CreateStore(argToAlloc, alloc);
        d->scope.push(arg->getName().getValue(), alloc);
    }

    for (Statement const* stat: node.getBody()) {
        stat->emit(this);
    }

    verifyFunction(*func);

    d->scope.leave();
}

void BitcodeEmitter::emit(Module const& node) {}

void BitcodeEmitter::emit(Program const& program) {
    for (Function const* function: program.getFunctions()) {
        function->emit(this);
    }

    if (program.getMain()) {
        program.getMain()->emit(this);
    }

    // TODO modules
}

void BitcodeEmitter::emit(Id const& node) {
    auto value = d->scope.lookup(node.getValue());

    if (!value) {
        // TODO variable not defined
    }

    d->retval = d->builder.CreateLoad(*value, node.getValue().c_str());
}

void BitcodeEmitter::emit(Integer const& node) {
    d->retval = llvm::ConstantInt::get(
        getGlobalContext(), llvm::APInt(64, node.getValue(), true)
    );
}

void BitcodeEmitter::emit(Float const& node) {
    d->retval = llvm::ConstantFP::get(
        getGlobalContext(), llvm::APFloat(node.getValue())
    );
}

static inline
bool isFP(llvm::Value const* var) {
    llvm::Type *type = var->getType();
    return type->isFloatTy() || type->isDoubleTy();
}

#define HANDLE(intop, fpop) \
    if (fp) { \
        d->retval = d->builder.Create##fpop(left, right); \
    } else { \
        d->retval = d->builder.Create##intop(left, right); \
    }

#define HANDLE_INT_ONLY(op) \
    if (fp) { \
        d->retval = nullptr; \
    } else { \
        d->retval = d->builder.Create##op(left, right); \
    }

static
void createOp(BitcodeEmitter::Private *d, llvm::Value *left, Operator op, llvm::Value *right) {
    bool fp = isFP(left) || isFP(right);

    // TODO left->getType() != right->getType() Handle automatic casts

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
            HANDLE_INT_ONLY(Shl);
            break;
        case Operator::SHR:
            HANDLE_INT_ONLY(LShr);
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
}

#undef HANDLE
#undef HANDLE_INT_ONLY

void BitcodeEmitter::emit(BinaryExpression const& expression) {
    expression.getLeft().emit(this);
    llvm::Value *left = d->retval;

    expression.getRight().emit(this);
    llvm::Value *right = d->retval;

    createOp(d, left, expression.getOperator(), right);
}

void BitcodeEmitter::emitSemiExpression(Id const& left, SemiExpression const& right) {
    left.emit(this);
    llvm::Value *lhs = d->retval;

    right.getLeft().emit(this);
    llvm::Value *rhs = d->retval;

    createOp(d, lhs, right.getOperator(), rhs);
}

