// Copyright 2017 the Monicelli project authors. All rights reserved.
// Use of this source code is governed by a GPLv3 license, see LICENSE.txt.

#include "codegen.def"
#include "ast-visitor.h"
#include "parser.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_os_ostream.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Utils.h"

#include <vector>

using namespace monicelli;

namespace {

class NestedScopes final {
public:
  class Guard final {
  public:
    Guard(NestedScopes& context) : context_(context) { context_.enterScope(); }

    ~Guard() { context_.leaveScope(); }

  private:
    NestedScopes& context_;
  };

  NestedScopes() {}

  NestedScopes(NestedScopes&) = delete;
  NestedScopes& operator=(NestedScopes&) = delete;

  llvm::Value* lookup(const std::string& name);

  bool define(const std::string& name, llvm::Value* def) {
    assert(!scopes_.empty() && "Trying to define outside any scope");
    auto result = scopes_.back().insert({name, def});
    return result.second;
  }

  void enterScope() { scopes_.emplace_back(); }

  void leaveScope() {
    assert(!scopes_.empty() && "Trying to leave a scope, but there is none");
    scopes_.pop_back();
  }

  void reset() { scopes_.clear(); }
  bool empty() const { return scopes_.empty(); }

private:
  std::vector<llvm::StringMap<llvm::Value*>> scopes_;
};

class IRGenerator;

class ResultTypeCalculator : public ConstAstVisitor<ResultTypeCalculator, llvm::Type*>,
                             public ErrorReportingMixin {
public:
  ResultTypeCalculator(IRGenerator* codegen, const std::string& source_filename)
      : ErrorReportingMixin(source_filename), codegen_(codegen) {}

  llvm::Type* visitBinaryExpression(const BinaryExpression* e);
  llvm::Type* visitAtomicExpression(const AtomicExpression* e);
  llvm::Type* visitFunctionCallExpression(const FunctionCallExpression* e);

private:
  IRGenerator* codegen_;
};

class IRGenerator final : public ConstAstVisitor<IRGenerator, llvm::Value*>,
                          public ErrorReportingMixin {
public:
  IRGenerator(llvm::LLVMContext& context, const std::string& source_filename)
      : ErrorReportingMixin(source_filename), context_(context), builder_(context),
        exit_block_(nullptr), return_var_(nullptr), type_calculator_(this, source_filename) {}

  std::unique_ptr<llvm::Module> releaseModule() { return std::move(module_); }
  llvm::Module* getModule() { return module_.get(); }

  llvm::Value* visitModule(const Module* m);
  llvm::Value* visitFunction(const Function* f);
  llvm::Value* visitVardeclStatement(const VardeclStatement* s);
  llvm::Value* visitReturnStatement(const ReturnStatement* r);
  llvm::Value* visitAssignStatement(const AssignStatement* a);
  llvm::Value* visitBranchStatement(const BranchStatement* b);
  llvm::Value* visitLoopStatement(const LoopStatement* l);
  llvm::Value* visitInputStatement(const InputStatement* s);
  llvm::Value* visitPrintStatement(const PrintStatement* p);
  llvm::Value* visitAssertStatement(const AssertStatement* a);
  llvm::Value* visitAbortStatement(const AbortStatement* a);
  llvm::Value* visitExpressionStatement(const ExpressionStatement* s) {
    visit(s->getExpression());
    return nullptr;
  }
  llvm::Value* visitBinaryExpression(const BinaryExpression* e);
  llvm::Value* visitAtomicExpression(const AtomicExpression* e);
  llvm::Value* visitFunctionCallExpression(const FunctionCallExpression* f);

private:
  llvm::Function* declareFunction(const Function* f);
  std::string getFunctionName(const Function* f) {
    return f->isEntryPoint() ? "main" : f->getName();
  }

  void declareBuiltins();

  template<bool output> const char* getFormatSpecifier(llvm::Type* type);
  template<bool output> llvm::Value* getFormatString(llvm::Type* type);
  template<bool output> void callIOBuiltin(llvm::Type* type, llvm::Value* value);

  llvm::Type* getIRType(const VarType& type) {
    auto base_type = getIRBaseType(type.getBaseType());
    if (type.isPointer()) {
      return base_type->getPointerTo();
    }
    return base_type;
  }

  llvm::Type* getIRBaseType(VarType::BaseType type);
  llvm::Value* ensureType(llvm::Value* value, llvm::Type* type);
  const char* getSourceBaseType(llvm::Type* type);
  std::string getSourceType(llvm::Type* type);

  llvm::Value* evalBooleanCondition(const Expression* condition_expression);
  llvm::Value* evalTruthiness(llvm::Value* val);

  llvm::Function* current_function() { return builder_.GetInsertBlock()->getParent(); }

  llvm::LLVMContext& context_;

  llvm::IRBuilder<> builder_;
  std::unique_ptr<llvm::Module> module_;

  NestedScopes var_scopes_;
  llvm::DenseMap<llvm::Type*, llvm::Value*> input_format_strings_cache_;
  llvm::DenseMap<llvm::Type*, llvm::Value*> output_format_strings_cache_;
  llvm::BasicBlock* exit_block_;
  llvm::AllocaInst* return_var_;

  ResultTypeCalculator type_calculator_;

  friend class ResultTypeCalculator;
};

} // namespace

llvm::Value* NestedScopes::lookup(const std::string& name) {
  for (auto c = scopes_.crbegin(), end = scopes_.crend(); c != end; ++c) {
    auto result = c->find(name);
    if (result != c->end()) return result->second;
  }
  return nullptr;
}

void IRGenerator::declareBuiltins() {
  llvm::FunctionType* abort_type = llvm::FunctionType::get(builder_.getVoidTy(), false);
  auto no_return = llvm::AttributeList().addAttribute(context_, 1, llvm::Attribute::NoReturn);
  module_->getOrInsertFunction("abort", abort_type, no_return);

  llvm::FunctionType* printf_type =
      llvm::FunctionType::get(builder_.getInt32Ty(), {builder_.getInt8PtrTy()}, true);
  auto no_alias = llvm::AttributeList().addAttribute(context_, 1, llvm::Attribute::NoAlias);

  module_->getOrInsertFunction("printf", printf_type, no_alias);
  module_->getOrInsertFunction("scanf", printf_type, no_alias);
}

llvm::Value* IRGenerator::visitModule(const Module* m) {
  module_ = llvm::make_unique<llvm::Module>("antani", context_);

  declareBuiltins();

  for (const Function* f : m->functions()) {
    declareFunction(f);
  }
  if (m->hasEntryPoint()) declareFunction(m->getEntryPoint());

  for (const Function* f : m->functions()) {
    visit(f);
  }
  if (m->hasEntryPoint()) visit(m->getEntryPoint());

  llvm::verifyModule(*module_);

  return nullptr;
}

llvm::Function* IRGenerator::declareFunction(const Function* ast_f) {
  std::vector<llvm::Type*> param_types;
  param_types.reserve(ast_f->params_size());
  for (const FunctionParam& param : ast_f->params()) {
    param_types.push_back(getIRType(param.getType()));
  }
  auto type = llvm::FunctionType::get(getIRType(ast_f->getReturnType()), param_types, false);

  llvm::Function* f = llvm::Function::Create(type, llvm::Function::ExternalLinkage,
                                             getFunctionName(ast_f), module_.get());

  auto ast_arg = ast_f->begin_params();
  for (auto& arg : f->args()) {
    arg.setName(ast_arg->getArg().getName());
    ++ast_arg;
  }
  assert(ast_arg == ast_f->end_params());

  return f;
}

llvm::Value* IRGenerator::visitFunction(const Function* ast_f) {
  llvm::Function* f = module_->getFunction(getFunctionName(ast_f));
  assert(f && "This function should have had a prototype defined");

  if (ast_f->body_empty()) return f;

  NestedScopes::Guard scopes_guard{var_scopes_};
  llvm::BasicBlock* entry = llvm::BasicBlock::Create(context_, "entry", f);
  builder_.SetInsertPoint(entry);

  if (!f->getReturnType()->isVoidTy()) {
    return_var_ = builder_.CreateAlloca(f->getReturnType(), nullptr, "result");
    if (ast_f->isEntryPoint()) {
      assert(f->getReturnType()->isIntegerTy());
      builder_.CreateStore(ensureType(builder_.getInt64(0), f->getReturnType()), return_var_);
    }
  } else {
    return_var_ = nullptr;
  }

  for (auto& arg : f->args()) {
    auto arg_ptr = builder_.CreateAlloca(arg.getType(), nullptr, arg.getName());
    builder_.CreateStore(&arg, arg_ptr);
    var_scopes_.define(arg.getName(), arg_ptr);
  }

  exit_block_ = llvm::BasicBlock::Create(context_, "exit");

  for (const Statement* s : ast_f->body()) {
    visit(s);
  }

  builder_.CreateBr(exit_block_);

  f->getBasicBlockList().push_back(exit_block_);
  builder_.SetInsertPoint(exit_block_);

  if (return_var_) {
    builder_.CreateRet(builder_.CreateLoad(return_var_));
  } else {
    builder_.CreateRetVoid();
  }

  llvm::verifyFunction(*f);

  exit_block_ = nullptr;
  return_var_ = nullptr;

  return f;
}

llvm::Value* IRGenerator::visitVardeclStatement(const VardeclStatement* s) {
  const auto& name = s->getVariable().getName();
  llvm::AllocaInst* var = builder_.CreateAlloca(getIRType(s->getType()), nullptr, name);
  if (!var_scopes_.define(name, var)) {
    error(&s->getVariable(), "redefining an existing variable");
  }
  if (s->hasInitializer()) {
    llvm::Value* init = visit(s->getInitializer());
    auto original_init_type = init->getType();
    auto target_type = var->getType()->getPointerElementType();
    init = ensureType(init, target_type);
    if (!init) {
      error(s->getInitializer(), "cannot initialize variable of type", getSourceType(target_type),
            "with expression of type", getSourceType(original_init_type));
    }
    builder_.CreateStore(init, var);
  }
  return var;
}

llvm::Value* IRGenerator::visitReturnStatement(const ReturnStatement* r) {
  if (r->hasExpression()) {
    auto return_value = visit(r->getExpression());
    auto original_return_type = return_value->getType();
    auto return_type = return_var_->getType()->getPointerElementType();
    return_value = ensureType(return_value, return_type);
    if (!return_value) {
      error(r->getExpression(), "cannot return expression of type", original_return_type,
            "from function of type", return_type);
    }
    builder_.CreateStore(return_value, return_var_);
  }
  builder_.CreateBr(exit_block_);
  // Code after the return will end up in this unreachable BB and DCE will
  // take care of it.
  llvm::BasicBlock* after = llvm::BasicBlock::Create(context_, "return.after", current_function());
  builder_.SetInsertPoint(after);
  // This one is not necessary, but will help catch codegen errors.
  builder_.CreateUnreachable();
  return nullptr;
}

llvm::Value* IRGenerator::visitAssignStatement(const AssignStatement* a) {
  auto val = visit(a->getExpression());
  assert(val && "unhandled error while building expression");
  auto var = var_scopes_.lookup(a->getVariable().getName());
  if (!var) {
    error(&a->getVariable(), "assigning to undefined variable", a->getVariable().getName());
  }
  auto original_val_type = val->getType();
  auto target_type = var->getType()->getPointerElementType();
  val = ensureType(val, target_type);
  if (!val) {
    error(a->getExpression(), "cannot assign expression of type", getSourceType(original_val_type),
          "to variable of type", getSourceType(target_type));
  }
  builder_.CreateStore(val, var);
  return nullptr;
}

llvm::Value* IRGenerator::evalBooleanCondition(const Expression* condition_expression) {
  llvm::Value* condition = visit(condition_expression);
  auto condition_type = condition->getType();
  condition = evalTruthiness(condition);
  if (!condition) {
    error(condition_expression, "cannot convert expression of type", getSourceType(condition_type),
          "to boolean.");
  }
  return condition;
}

llvm::Value* IRGenerator::evalTruthiness(llvm::Value* val) {
  if (llvm::isa<llvm::CmpInst>(val)) return val;
  auto val_type = val->getType();
  if (!val_type->isIntegerTy() && !val_type->isFloatingPointTy()) {
    return nullptr;
  }
  auto zero = ensureType(builder_.getInt64(0), val_type);
  return builder_.CreateICmpNE(val, zero, "cond");
}

llvm::Value* IRGenerator::visitBranchStatement(const BranchStatement* b) {
  llvm::BasicBlock* entry_bb =
      llvm::BasicBlock::Create(context_, "branch.head", current_function());
  llvm::BasicBlock* exit_bb = llvm::BasicBlock::Create(context_, "branch.after");

  builder_.CreateBr(entry_bb);
  builder_.SetInsertPoint(entry_bb);

  llvm::BasicBlock* case_cond_bb =
      llvm::BasicBlock::Create(context_, "branch.case.cond", current_function());

  builder_.CreateBr(case_cond_bb);
  builder_.SetInsertPoint(case_cond_bb);

  for (const BranchCase& branch_case : b->cases()) {
    llvm::Value* condition = evalBooleanCondition(branch_case.getExpression());

    case_cond_bb = llvm::BasicBlock::Create(context_, "branch.case.cond");
    llvm::BasicBlock* case_body_bb =
        llvm::BasicBlock::Create(context_, "branch.case.body", current_function());

    builder_.CreateCondBr(condition, case_body_bb, case_cond_bb);
    builder_.SetInsertPoint(case_body_bb);
    for (const Statement* s : branch_case.body()) {
      visit(s);
    }
    builder_.CreateBr(exit_bb);
    current_function()->getBasicBlockList().push_back(case_cond_bb);
    builder_.SetInsertPoint(case_cond_bb);
  }

  if (b->hasBranchElse()) {
    NestedScopes::Guard scope_guard{var_scopes_};
    llvm::BasicBlock* else_bb =
        llvm::BasicBlock::Create(context_, "branch.else", current_function());
    builder_.CreateBr(else_bb);
    builder_.SetInsertPoint(else_bb);
    for (const Statement* s : b->getBranchElse()->body()) {
      visit(s);
    }
  }

  builder_.CreateBr(exit_bb);
  current_function()->getBasicBlockList().push_back(exit_bb);
  builder_.SetInsertPoint(exit_bb);

  return nullptr;
}

llvm::Value* IRGenerator::visitLoopStatement(const LoopStatement* l) {
  llvm::BasicBlock* body_bb = llvm::BasicBlock::Create(context_, "loop.body", current_function());
  builder_.CreateBr(body_bb);
  builder_.SetInsertPoint(body_bb);

  {
    NestedScopes::Guard scope_guard{var_scopes_};
    for (const Statement* s : l->body()) {
      visit(s);
    }
  }

  llvm::BasicBlock* condition_bb =
      llvm::BasicBlock::Create(context_, "loop.condition", current_function());
  llvm::BasicBlock* after_bb = llvm::BasicBlock::Create(context_, "loop.after");

  builder_.CreateBr(condition_bb);
  builder_.SetInsertPoint(condition_bb);
  llvm::Value* condition = evalBooleanCondition(l->getCondition());
  builder_.CreateCondBr(condition, body_bb, after_bb);

  current_function()->getBasicBlockList().push_back(after_bb);
  builder_.SetInsertPoint(after_bb);

  return nullptr;
}

const char* IRGenerator::getSourceBaseType(llvm::Type* type) {
  assert(type->isIntegerTy() || type->isFloatingPointTy());
#define RETURN_BASE_NAME(_1, TYPE, _2, _3, SOURCE_NAME, _4) \
  if (type == builder_.get##TYPE##Ty()) { \
    return SOURCE_NAME; \
  }
  BUILTIN_TYPES(RETURN_BASE_NAME)
#undef RETURN_BASE_NAME

  UNREACHABLE("Unhandled base type.");
}

std::string IRGenerator::getSourceType(llvm::Type* type) {
  std::string name;
  while (type->isPointerTy()) {
    name += "conte ";
    type = type->getPointerElementType();
  }
  name += getSourceBaseType(type);
  return name;
}

namespace {

template<> const char* IRGenerator::getFormatSpecifier<false>(llvm::Type* type) {
  assert(type->isIntegerTy() || type->isFloatingPointTy());
#define RETURN_SPECIFIER(_1, TYPE, SPEC, _2, _3, _4) \
  if (type == builder_.get##TYPE##Ty()) { \
    return SPEC; \
  }
  BUILTIN_TYPES(RETURN_SPECIFIER)
#undef RETURN_SPECIFIER

  UNREACHABLE("Unhandled input format specifier");
}

template<> const char* IRGenerator::getFormatSpecifier<true>(llvm::Type* type) {
  assert(type->isIntegerTy() || type->isFloatingPointTy());
#define RETURN_SPECIFIER(_1, TYPE, _2, SPEC, _3, _4) \
  if (type == builder_.get##TYPE##Ty()) { \
    return SPEC; \
  }
  BUILTIN_TYPES(RETURN_SPECIFIER)
#undef RETURN_SPECIFIER

  UNREACHABLE("Unhandled output format specifier");
}

} // namespace

template<bool output> llvm::Value* IRGenerator::getFormatString(llvm::Type* type) {
  auto& cache = output ? output_format_strings_cache_ : input_format_strings_cache_;
  auto hit = cache.find(type);
  if (hit != cache.end()) return hit->second;
  auto specifier = getFormatSpecifier<output>(type);
  auto format = builder_.CreateGlobalStringPtr(specifier, "format");
  cache.insert({type, format});
  return format;
}

template<bool output> void IRGenerator::callIOBuiltin(llvm::Type* type, llvm::Value* value) {
  auto builtin = module_->getFunction(output ? "printf" : "scanf");
  assert(builtin && "Builtin was not declared");
  llvm::Value* args[] = {getFormatString<output>(type), value};
  builder_.CreateCall(builtin, args);
}

llvm::Value* IRGenerator::visitInputStatement(const InputStatement* s) {
  auto var = var_scopes_.lookup(s->getVariable().getName());
  if (!var) {
    error(&s->getVariable(), "reading an undefined variable");
  }
  assert(var->getType()->isPointerTy());

  auto target = var;
  auto target_type = target->getType()->getPointerElementType();
  bool reading_bool = target_type == builder_.getInt1Ty();
  if (!target_type->isIntegerTy() && !target_type->isFloatingPointTy()) {
    error(&s->getVariable(), "can only read integers and floating point");
  }

  if (reading_bool) {
    target = builder_.CreateAlloca(builder_.getInt32Ty());
  }

  callIOBuiltin<false>(target_type, target);

  if (reading_bool) {
    builder_.CreateStore(evalTruthiness(builder_.CreateLoad(target)), var);
  }

  return nullptr;
}

llvm::Value* IRGenerator::visitPrintStatement(const PrintStatement* p) {
  auto value = visit(p->getExpression());
  auto type = value->getType();
  if (!type->isIntegerTy() && !type->isFloatingPointTy()) {
    error(p->getExpression(), "only integer and float valued expressions may be printed");
  }
  // Integer promotion for variadic call.
  if (type->isIntegerTy() && type->getIntegerBitWidth() < 32) {
    value = builder_.CreateZExt(value, builder_.getInt32Ty());
  }
  // Same, for floating point.
  if (type == builder_.getFloatTy()) {
    value = builder_.CreateFPCast(value, builder_.getDoubleTy());
  }
  callIOBuiltin<true>(type, value);
  return nullptr;
}

llvm::Value* IRGenerator::visitAssertStatement(const AssertStatement* a) {
  llvm::BasicBlock* check_bb =
      llvm::BasicBlock::Create(context_, "assert.check", current_function());
  llvm::BasicBlock* fail_bb = llvm::BasicBlock::Create(context_, "assert.fail", current_function());
  llvm::BasicBlock* success_bb =
      llvm::BasicBlock::Create(context_, "assert.success", current_function());

  builder_.CreateBr(check_bb);

  builder_.SetInsertPoint(check_bb);
  llvm::Value* condition = evalBooleanCondition(a->getExpression());
  builder_.CreateCondBr(condition, success_bb, fail_bb);

  builder_.SetInsertPoint(fail_bb);
  visitAbortStatement(nullptr);  // Assert is just a conditional abort.
  builder_.CreateBr(success_bb); // We will never get here, but LLVM does not know.

  builder_.SetInsertPoint(success_bb);
  return nullptr;
}

llvm::Value* IRGenerator::visitAbortStatement(const AbortStatement*) {
  auto abort_builtin = module_->getFunction("abort");
  assert(abort_builtin && "Builtin abort was not declared");
  builder_.CreateCall(abort_builtin);
  return nullptr;
}

llvm::Value* IRGenerator::visitBinaryExpression(const BinaryExpression* e) {
  auto lhs = visit(e->getLeft());
  auto rhs = visit(e->getRight());

  if (lhs->getType()->isPointerTy() || rhs->getType()->isPointerTy()) {
    error(e, "pointer arithmetic is not supported");
  }

  llvm::Type* result_type = type_calculator_.visit(e);
  auto original_lhs_type = lhs->getType();
  auto original_rhs_type = rhs->getType();
  lhs = ensureType(lhs, result_type);
  rhs = ensureType(rhs, result_type);

  if (!lhs || !rhs) {
    auto expression = !lhs ? e->getLeft() : e->getRight();
    error(expression, "cannot cast expression of type", getSourceType(original_lhs_type), "to type",
          getSourceType(original_rhs_type));
  }

  if (result_type->isIntegerTy()) {
    switch (e->getType()) {
#define RETURN_INT_BINOP(NAME, OP) \
  case BinaryExpression::NAME: \
    return builder_.CreateBinOp(llvm::Instruction::OP, lhs, rhs);
      IR_INT_BINARY_OPS(RETURN_INT_BINOP)
#undef RETURN_INT_BINOP
#define RETURN_INT_CMPOP(NAME, OP) \
  case BinaryExpression::NAME: \
    return builder_.CreateICmp(llvm::CmpInst::ICMP_##OP, lhs, rhs);
      IR_INT_CMP_OPS(RETURN_INT_CMPOP)
#undef RETURN_INT_CMPOP
    default:
      error(e, "this operation cannot be applied to integers");
    }

    return nullptr;
  }

  if (result_type->isFloatingPointTy()) {
    switch (e->getType()) {
#define RETURN_FLOAT_BINOP(NAME, OP) \
  case BinaryExpression::NAME: \
    return builder_.CreateBinOp(llvm::Instruction::OP, lhs, rhs);
      IR_FLOAT_BINARY_OPS(RETURN_FLOAT_BINOP)
#undef RETURN_FLOAT_BINOP
#define RETURN_FLOAT_CMPOP(NAME, OP) \
  case BinaryExpression::NAME: \
    return builder_.CreateFCmp(llvm::CmpInst::FCMP_##OP, lhs, rhs);
      IR_FLOAT_CMP_OPS(RETURN_FLOAT_CMPOP)
#undef RETURN_FLOAT_CMPOP
    default:
      error(e, "this operation cannot be applied to floats");
    }

    return nullptr;
  }

  UNREACHABLE("Unimplemented operand conversion");
}

llvm::Value* IRGenerator::visitAtomicExpression(const AtomicExpression* e) {
  switch (e->getType()) {
  case AtomicExpression::INTEGER:
    return builder_.getInt32(e->getIntValue());
  case AtomicExpression::FLOAT:
    return llvm::ConstantFP::get(builder_.getDoubleTy(), e->getFloatValue());
  case AtomicExpression::IDENTIFIER: {
    auto var = var_scopes_.lookup(e->getIdentifierValue().getName());
    if (!var) {
      error(&e->getIdentifierValue(), "undefined variable", e->getIdentifierValue().getName());
    }
    assert(llvm::isa<llvm::AllocaInst>(var));
    return builder_.CreateLoad(var);
  }
  default:
    UNREACHABLE("Unhandled AtomicExpression type");
  }
}

llvm::Value* IRGenerator::visitFunctionCallExpression(const FunctionCallExpression* ast_f) {
  llvm::Function* f = module_->getFunction(ast_f->getFunctionName());
  if (!f) {
    error(ast_f, "call to undefined function", ast_f->getFunctionName());
  }
  std::vector<llvm::Value*> call_args;
  auto ir_arg = f->arg_begin();
  for (const Expression* ast_arg : ast_f->args()) {
    auto arg = visit(ast_arg);
    auto original_arg_type = arg->getType();
    arg = ensureType(arg, ir_arg->getType());
    if (!arg) {
      error(ast_arg, "cannot pass expression of type", getSourceType(original_arg_type),
            "as argument of type", getSourceType(ir_arg->getType()), "in call to",
            ast_f->getFunctionName());
    }
    call_args.push_back(arg);
    ++ir_arg;
  }
  assert(ir_arg == f->arg_end());
  return builder_.CreateCall(f, call_args);
}

llvm::Type* IRGenerator::getIRBaseType(VarType::BaseType type) {
  switch (type) {
#define RETURN_IR_TYPE(NAME, IR_TYPE, _1, _2, _3, _4) \
  case VarType::NAME: \
    return builder_.get##IR_TYPE##Ty();
    BUILTIN_TYPES(RETURN_IR_TYPE)
#undef RETURN_IR_TYPE
  default:
    UNREACHABLE("Unhandled VarType::BaseType on getIRBaseType");
  }
}

llvm::Value* IRGenerator::ensureType(llvm::Value* value, llvm::Type* type) {
  if (value->getType()->isPointerTy() != type->isPointerTy()) {
    // Cannot cast pointer to int or viceversa.
    return nullptr;
  }
  if (value->getType()->isPointerTy()) {
    // Pointers are not castable.
    return value->getType() == type ? value : nullptr;
  }
  if (value->getType()->isIntegerTy() && type->isIntegerTy()) {
    return builder_.CreateSExtOrTrunc(value, type);
  }
  if (value->getType()->isFloatingPointTy() && type->isIntegerTy()) {
    return builder_.CreateFPToSI(value, type);
  }
  if (value->getType()->isIntegerTy() && type->isFloatingPointTy()) {
    return builder_.CreateSIToFP(value, type);
  }
  if (value->getType()->isFloatingPointTy() && type->isFloatingPointTy()) {
    return builder_.CreateFPCast(value, type);
  }
  UNREACHABLE("Unhandled IR type conversion");
}

llvm::Type* ResultTypeCalculator::visitBinaryExpression(const BinaryExpression* e) {
  llvm::Type* ltype = visit(e->getLeft());
  llvm::Type* rtype = visit(e->getRight());
  // Void should not be here at all.
  if (ltype->isVoidTy() || rtype->isVoidTy()) {
    error(e, "cannot operate on void");
  }
  // Same type, job done.
  if (ltype == rtype) return ltype;
  // Pointers are not castable.
  if (ltype->isPointerTy() != rtype->isPointerTy()) {
    error(e, "cannot cast pointer to int");
  }
  if (ltype->isPointerTy() && rtype->isPointerTy()) {
    // implied: different pointer types.
    error(e, "cannot cast between pointer types");
  }
  // Double (floating point) always wins.
  if (ltype->isFloatingPointTy() || rtype->isFloatingPointTy()) {
    return codegen_->builder_.getDoubleTy();
  }
  // Integers always upcast.
  if (ltype->isIntegerTy() && rtype->isIntegerTy()) {
    int lsize = ltype->getPrimitiveSizeInBits();
    int rsize = rtype->getPrimitiveSizeInBits();
    return lsize > rsize ? ltype : rtype;
  }

  UNREACHABLE("Unhandled BinaryExpression type");
}

llvm::Type* ResultTypeCalculator::visitAtomicExpression(const AtomicExpression* e) {
  switch (e->getType()) {
  case AtomicExpression::INTEGER:
    return codegen_->builder_.getInt32Ty();
  case AtomicExpression::FLOAT:
    return codegen_->builder_.getDoubleTy();
  case AtomicExpression::IDENTIFIER: {
    auto var = codegen_->var_scopes_.lookup(e->getIdentifierValue().getName());
    assert(var);
    return var->getType()->getPointerElementType();
  }
  default:
    UNREACHABLE("Unhandled AtomicExpression type");
  }
}

llvm::Type* ResultTypeCalculator::visitFunctionCallExpression(const FunctionCallExpression* e) {
  auto f = codegen_->module_->getFunction(e->getFunctionName());
  assert(f);
  return f->getReturnType();
}

namespace monicelli {

std::unique_ptr<llvm::Module> generateIR(llvm::LLVMContext& context, Module* ast) {
  IRGenerator codegen{context, ast->getSourceFilename()};
  codegen.visit(ast);
  return codegen.releaseModule();
}

void runFunctionOptimizer(llvm::Module* module) {
  llvm::legacy::FunctionPassManager pass_manager{module};
  pass_manager.add(llvm::createInstructionCombiningPass());
  pass_manager.add(llvm::createReassociatePass());
  pass_manager.add(llvm::createGVNPass());
  pass_manager.add(llvm::createCFGSimplificationPass());
  pass_manager.add(llvm::createDeadCodeEliminationPass());
  pass_manager.add(llvm::createPromoteMemoryToRegisterPass());
  pass_manager.doInitialization();
  for (llvm::Function& f : module->functions()) {
    pass_manager.run(f);
  }
}

void printIR(std::ostream& stream, llvm::Module* module) {
  llvm::raw_os_ostream llvm_stream{stream};
  module->print(llvm_stream, nullptr);
}

} // namespace monicelli
