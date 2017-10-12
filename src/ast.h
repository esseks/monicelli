#ifndef MONICELLI_AST_H
#define MONICELLI_AST_H

// Copyright 2017 the Monicelli project authors. All rights reserved.
// Use of this source code is governed by a GPLv3 license, see LICENSE.txt.

#include "ast.def"
#include "iterators.h"
#include "location.h"

#include <cassert>
#include <memory>
#include <string>
#include <vector>

namespace monicelli {

class Variable final : public LocationMixin {
public:
  const std::string& getName() const { return name_; }

private:
  std::string name_;

  friend class Parser;
};

class VarType final {
public:
  enum BaseType {
#define DECLARE_TYPE(NAME, _1, _2, _3, _4, _5) NAME,
    BUILTIN_TYPES(DECLARE_TYPE)
#undef DECLARE_TYPE
  };

  VarType() : base_type_(VarType::VOID), pointer_(false) {}

  bool isVoid() const { return base_type_ == BaseType::VOID && !pointer_; }
  BaseType getBaseType() const { return base_type_; }
  bool isPointer() const { return pointer_; }

private:
  BaseType base_type_;
  bool pointer_;

  friend class Parser;
};

class FunctionParam final {
public:
  FunctionParam(const Variable& name, const VarType& type) : name_(name), type_(type) {}

  const Variable& getArg() const { return name_; }
  VarType getType() const { return type_; }

private:
  Variable name_;
  VarType type_;

  friend class Parser;
};

class AstNode {
public:
  enum ClassType : uint8_t {
#define DECL_EXPR_TYPE(NAME) TYPE_##NAME,
    AST_NODES(DECL_EXPR_TYPE)
#undef DECL_EXPR_TYPE
  };

  ClassType getClassType() const { return type_tag_; }

protected:
  AstNode(ClassType type_tag) : type_tag_(type_tag) {}

private:
  ClassType type_tag_;
};

class Expression : public AstNode, public LocationMixin {
public:
  Expression(Expression::ClassType type) : AstNode(type) {}
  virtual ~Expression() = default;

  bool isFunctionCall() const { return getClassType() == Expression::TYPE_FunctionCallExpression; }
};

class BinaryExpression final : public Expression {
public:
  enum Type {
#define DECLARE_OP(_, NAME, __, ___) NAME,
    AST_BINARY_OPERATORS(DECLARE_OP)
#undef DECLARE_OP
  };

  Type getType() const { return type_; }
  bool isSemiExpression() const { return is_semi_; }
  const Expression* getLeft() const { return left_.get(); }
  const Expression* getRight() const { return right_.get(); }

  static const char* getOperatorRepresentation(BinaryExpression::Type type);

  const char* getOperatorRepresentation() const { return getOperatorRepresentation(type_); }

private:
  BinaryExpression(Type type, std::shared_ptr<Expression> left, Expression* right, bool is_semi)
      : Expression(Expression::TYPE_BinaryExpression), type_(type), is_semi_(is_semi), left_(left),
        right_(right) {}

  Type type_;
  bool is_semi_;
  std::shared_ptr<Expression> left_;
  std::unique_ptr<Expression> right_;

  friend class Parser;
};

class AtomicExpression final : public Expression {
public:
  enum Type { IDENTIFIER, INTEGER, FLOAT };

  ~AtomicExpression() override {
    if (type_ == Type::IDENTIFIER) {
      identifier_value_.~Variable();
    }
  }

  Type getType() const { return type_; }

  uint64_t getIntValue() const {
    assert(type_ == Type::INTEGER);
    return int_value_;
  }

  double getFloatValue() const {
    assert(type_ == Type::FLOAT);
    return fp_value_;
  }

  const Variable& getIdentifierValue() const {
    assert(type_ == Type::IDENTIFIER);
    return identifier_value_;
  }

private:
  AtomicExpression() : Expression(Expression::TYPE_AtomicExpression) {}

  static std::unique_ptr<AtomicExpression> fromInt(uint64_t value) {
    std::unique_ptr<AtomicExpression> expression{new AtomicExpression};
    expression->type_ = Type::INTEGER;
    expression->int_value_ = value;
    return expression;
  }

  static std::unique_ptr<AtomicExpression> fromFloat(double value) {
    std::unique_ptr<AtomicExpression> expression{new AtomicExpression};
    expression->type_ = Type::FLOAT;
    expression->fp_value_ = value;
    return expression;
  }

  static std::unique_ptr<AtomicExpression> fromIdentifier(const Variable& value) {
    std::unique_ptr<AtomicExpression> expression{new AtomicExpression};
    expression->type_ = Type::IDENTIFIER;
    new (&expression->identifier_value_) Variable{value};
    return expression;
  }

  Type type_;
  union {
    uint64_t int_value_;
    double fp_value_;
    Variable identifier_value_;
  };

  friend class Parser;
};

class Statement : public AstNode {
public:
  Statement(Statement::ClassType type) : AstNode(type) {}

  virtual ~Statement() = default;
};

class AssertStatement final : public Statement {
public:
  AssertStatement() : Statement(Statement::TYPE_AssertStatement) {}

  const Expression* getExpression() const { return expression_.get(); }

private:
  std::unique_ptr<Expression> expression_;

  friend class Parser;
};

class FunctionCallExpression final : public Expression {
public:
  typedef PointerVectorConstIter<Expression> FunctionArgsConstIter;

  FunctionCallExpression() : Expression(Expression::TYPE_FunctionCallExpression) {}

  const std::string& getFunctionName() const { return function_name_; }
  FunctionArgsConstIter args_begin() const { return function_args_.cbegin(); }
  FunctionArgsConstIter args_end() const { return function_args_.cend(); }
  ConstRangeWrapper<FunctionArgsConstIter> args() const { return {args_begin(), args_end()}; }

private:
  std::string function_name_;
  std::vector<std::unique_ptr<Expression>> function_args_;

  friend class Parser;
};

class ExpressionStatement final : public Statement {
public:
  ExpressionStatement() : Statement(Statement::TYPE_ExpressionStatement) {}

  const Expression* getExpression() const { return expression_.get(); }

private:
  std::unique_ptr<Expression> expression_;

  friend class Parser;
};

class InputStatement final : public Statement {
public:
  InputStatement() : Statement(Statement::TYPE_InputStatement) {}

  const Variable& getVariable() const { return variable_; }

private:
  Variable variable_;

  friend class Parser;
};

class AbortStatement final : public Statement {
public:
  AbortStatement() : Statement(Statement::TYPE_AbortStatement) {}

private:
  friend class Parser;
};

class BranchCase final {
public:
  typedef PointerVectorConstIter<Statement> BodyConstIter;

  const Expression* getExpression() const { return expression_.get(); }
  BodyConstIter begin_body() const { return body_.cbegin(); }
  BodyConstIter end_body() const { return body_.cend(); }
  ConstRangeWrapper<BodyConstIter> body() const { return {begin_body(), end_body()}; }

private:
  std::unique_ptr<Expression> expression_;
  std::vector<std::unique_ptr<Statement>> body_;

  friend class Parser;
};

class BranchElse final {
public:
  typedef PointerVectorConstIter<Statement> BodyConstIter;

  BodyConstIter begin_body() const { return body_.cbegin(); }
  BodyConstIter end_body() const { return body_.cend(); }
  ConstRangeWrapper<BodyConstIter> body() const { return {begin_body(), end_body()}; }

private:
  std::vector<std::unique_ptr<Statement>> body_;

  friend class Parser;
};

class BranchStatement final : public Statement {
public:
  typedef std::vector<BranchCase>::const_iterator BranchCaseConstIter;

  BranchStatement() : Statement(Statement::TYPE_BranchStatement) {}

  const Variable& getLeadVariable() const { return lead_var_; }

  BranchCaseConstIter begin_cases() const { return cases_.cbegin(); }
  BranchCaseConstIter end_cases() const { return cases_.cend(); }
  ConstRangeWrapper<BranchCaseConstIter> cases() const { return {begin_cases(), end_cases()}; }

  bool hasBranchElse() const { return static_cast<bool>(maybe_else_case_); }
  const BranchElse* getBranchElse() const {
    assert(hasBranchElse());
    return maybe_else_case_.get();
  }

private:
  Variable lead_var_;
  std::vector<BranchCase> cases_;
  std::unique_ptr<BranchElse> maybe_else_case_;

  friend class Parser;
};

class VardeclStatement final : public Statement {
public:
  VardeclStatement() : Statement(Statement::TYPE_VardeclStatement) {}

  const Variable& getVariable() const { return variable_; }
  const VarType& getType() const { return type_; }

  bool hasInitializer() const { return static_cast<bool>(maybe_init_); }
  const Expression* getInitializer() const {
    assert(hasInitializer());
    return maybe_init_.get();
  }

private:
  Variable variable_;
  VarType type_;
  std::unique_ptr<Expression> maybe_init_;

  friend class Parser;
};

class LoopStatement final : public Statement {
public:
  typedef PointerVectorConstIter<Statement> BodyConstIter;

  LoopStatement() : Statement(Statement::TYPE_LoopStatement) {}

  BodyConstIter begin_body() const { return body_.cbegin(); }
  BodyConstIter end_body() const { return body_.cend(); }
  ConstRangeWrapper<BodyConstIter> body() const { return {begin_body(), end_body()}; }

  const Expression* getCondition() const { return condition_.get(); }

private:
  std::vector<std::unique_ptr<Statement>> body_;
  std::unique_ptr<Expression> condition_;

  friend class Parser;
};

class ReturnStatement final : public Statement {
public:
  ReturnStatement() : Statement(Statement::TYPE_ReturnStatement) {}

  bool hasExpression() const { return static_cast<bool>(maybe_expression_); }
  const Expression* getExpression() const {
    assert(hasExpression());
    return maybe_expression_.get();
  }

private:
  std::unique_ptr<Expression> maybe_expression_;

  friend class Parser;
};

class PrintStatement final : public Statement {
public:
  PrintStatement() : Statement(Statement::TYPE_PrintStatement) {}

  const Expression* getExpression() const { return expression_.get(); }

private:
  std::unique_ptr<Expression> expression_;

  friend class Parser;
};

class AssignStatement final : public Statement {
public:
  AssignStatement() : Statement(Statement::TYPE_AssignStatement) {}

  const Variable& getVariable() const { return variable_; }
  const Expression* getExpression() const { return expression_.get(); }

private:
  std::unique_ptr<Expression> expression_;
  Variable variable_;

  friend class Parser;
};

class Function final : public AstNode {
public:
  Function() : AstNode(Statement::TYPE_Function) {}

  typedef std::vector<FunctionParam>::const_iterator FunctionParamConstIter;
  typedef PointerVectorConstIter<Statement> BodyConstIter;

  const std::string& getName() const { return name_; }
  const VarType& getReturnType() const { return return_type_; }
  bool isEntryPoint() const { return name_.empty(); }

  int params_size() const { return params_.size(); }
  FunctionParamConstIter begin_params() const { return params_.cbegin(); }
  FunctionParamConstIter end_params() const { return params_.cend(); }
  ConstRangeWrapper<FunctionParamConstIter> params() const {
    return {begin_params(), end_params()};
  }

  BodyConstIter begin_body() const { return body_.cbegin(); }
  BodyConstIter end_body() const { return body_.cend(); }
  bool body_empty() const { return body_.empty(); }
  ConstRangeWrapper<BodyConstIter> body() const { return {begin_body(), end_body()}; }

private:
  std::string name_;
  VarType return_type_;
  std::vector<FunctionParam> params_;
  std::vector<std::unique_ptr<Statement>> body_;

  friend class Parser;
};

class Module final : public AstNode {
public:
  typedef PointerVectorConstIter<Function> FunctionsConstIter;

  Module() : AstNode(AstNode::TYPE_Module) {}

  bool hasEntryPoint() const { return static_cast<bool>(maybe_entry_point_); }
  const Function* getEntryPoint() const {
    assert(hasEntryPoint());
    return maybe_entry_point_.get();
  }

  FunctionsConstIter begin_functions() const { return functions_.cbegin(); }
  FunctionsConstIter end_functions() const { return functions_.cend(); }
  ConstRangeWrapper<FunctionsConstIter> functions() const {
    return {begin_functions(), end_functions()};
  }

  const std::string& getSourceFilename() const { return source_filename_; }

private:
  std::vector<std::unique_ptr<Function>> functions_;
  std::unique_ptr<Function> maybe_entry_point_;
  std::string source_filename_;

  friend class Parser;
};

} // namespace monicelli

#endif
