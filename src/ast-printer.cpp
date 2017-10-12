// Copyright 2017 the Monicelli project authors. All rights reserved.
// Use of this source code is governed by a GPLv3 license, see LICENSE.txt.

#include "ast-visitor.h"
#include "ast.h"

#include <iostream>

using namespace monicelli;

namespace {

static const char* baseTypeToString(const VarType::BaseType type) {
  switch (type) {
#define RETURN_BASE_NAME(NAME, _1, _2, _3, _4, STRING) \
  case VarType::NAME: \
    return STRING;
    BUILTIN_TYPES(RETURN_BASE_NAME)
#undef RETURN_BASE_NAME
  default:
    UNREACHABLE("Unhandled VarType::BaseType.");
  }
}

static std::ostream& operator<<(std::ostream& stream, const VarType& type) {
  stream << baseTypeToString(type.getBaseType());
  if (type.isPointer()) {
    stream << '*';
  }
  return stream;
}

class AstPrinter final : public ConstAstVisitor<AstPrinter, void> {
public:
  AstPrinter(std::ostream& stream) : expression_level_(0), indent_level_(0), stream_(stream) {}

  void visitModule(const Module* module) {
    for (const Function* function : module->functions()) {
      visitFunction(function);
    }
    if (module->hasEntryPoint()) {
      visitFunction(module->getEntryPoint());
    }
  }

  void visitFunction(const Function* function) {
    stream() << function->getReturnType() << ' ';
    if (function->isEntryPoint()) {
      stream() << "main";
    } else {
      stream() << function->getName();
    }
    stream() << '(';

    bool first = true;
    for (const FunctionParam& param : function->params()) {
      if (first) {
        first = false;
      } else {
        stream() << ", ";
      }
      assert(!param.getType().isVoid());
      stream() << param.getType() << ' ' << param.getArg().getName();
    }

    stream() << ')';

    if (!function->body_empty()) {
      stream(false) << " {\n";
      IndentGuard guard(this);
      for (const Statement* statement : function->body()) {
        visit(statement);
        stream() << '\n';
      }
      stream(false) << "}";
    }

    stream(false) << "\n\n";
  }

  void visitAssertStatement(const AssertStatement* s) {
    stream() << "assert ";
    visit(s->getExpression());
  }

  void visitInputStatement(const InputStatement* s) {
    stream() << "read " << s->getVariable().getName();
  }

  void visitExpressionStatement(const ExpressionStatement* s) { visit(s->getExpression()); }

  void visitAbortStatement(const AbortStatement*) { stream() << "abort"; }

  void visitVardeclStatement(const VardeclStatement* s) {
    assert(!s->getType().isVoid());
    stream() << s->getType() << ' ' << s->getVariable().getName();
    if (s->hasInitializer()) {
      stream(false) << " = ";
      visit(s->getInitializer());
    }
  }

  void visitBranchStatement(const BranchStatement* branch) {
    stream() << "branch " << branch->getLeadVariable().getName() << " {\n";
    {
      IndentGuard guard(this);
      for (const BranchCase& c : branch->cases()) {
        stream() << "case";
        visit(c.getExpression());
        stream(false) << " {\n";
        {
          IndentGuard guard(this);
          for (const Statement* s : c.body()) {
            visit(s);
            stream(false) << '\n';
          }
        }
        stream() << "}\n";
      }
      if (branch->hasBranchElse()) {
        stream() << "else {\n";
        {
          IndentGuard guard(this);
          for (const Statement* s : branch->getBranchElse()->body()) {
            visit(s);
            stream(false) << '\n';
          }
        }
        stream() << "}\n";
      }
    }
    stream() << "}";
  }

  void visitLoopStatement(const LoopStatement* s) {
    stream() << "do {\n";
    {
      IndentGuard guard(this);
      for (const Statement* is : s->body()) {
        visit(is);
        stream(false) << '\n';
      }
    }
    stream() << "} while ";
    visit(s->getCondition());
  }

  void visitReturnStatement(const ReturnStatement* s) {
    ExpressionNestingGuard guard{this};
    stream() << "return";
    if (s->hasExpression()) {
      stream(false) << ' ';
      visit(s->getExpression());
    }
  }

  void visitPrintStatement(const PrintStatement* s) {
    ExpressionNestingGuard guard{this};
    stream() << "print ";
    visit(s->getExpression());
  }

  void visitAssignStatement(const AssignStatement* s) {
    ExpressionNestingGuard guard{this};
    stream() << s->getVariable().getName() << " = ";
    visit(s->getExpression());
  }

  void visitAtomicExpression(const AtomicExpression* s) {
    ExpressionNestingGuard guard{this};
    switch (s->getType()) {
    case AtomicExpression::FLOAT:
      stream(false) << s->getFloatValue();
      break;
    case AtomicExpression::INTEGER:
      stream(false) << s->getIntValue();
      break;
    case AtomicExpression::IDENTIFIER:
      stream(false) << s->getIdentifierValue().getName();
      break;
    default:
      UNREACHABLE("Unhanlded AtomicExpression type.");
    }
  }

  void visitBinaryExpression(const BinaryExpression* s) {
    if (isNestedExpression()) stream(false) << '(';
    {
      ExpressionNestingGuard guard{this};
      if (!s->isSemiExpression()) visit(s->getLeft());
      stream(false) << ' ' << s->getOperatorRepresentation() << ' ';
      visit(s->getRight());
    }
    if (isNestedExpression()) stream(false) << ')';
  }

  void visitFunctionCallExpression(const FunctionCallExpression* s) {
    stream(!isNestedExpression()) << s->getFunctionName() << '(';
    ExpressionNestingGuard guard{this};
    bool first = true;
    for (const Expression* arg : s->args()) {
      if (first) {
        first = false;
      } else {
        stream(false) << ", ";
      }
      visit(arg);
    }
    stream(false) << ')';
  }

private:
  class IndentGuard final {
  public:
    IndentGuard(AstPrinter* printer) : printer_(printer) { printer_->increaseIndent(); }

    ~IndentGuard() { printer_->decreaseIndent(); }

  private:
    AstPrinter* printer_;
  };

  class ExpressionNestingGuard final {
  public:
    ExpressionNestingGuard(AstPrinter* printer) : printer_(printer) {
      ++printer->expression_level_;
    }

    ~ExpressionNestingGuard() { --printer_->expression_level_; }

  private:
    AstPrinter* printer_;
  };

  bool isNestedExpression() const { return expression_level_ > 0; }

  void increaseIndent() { indent_level_ += 2; }
  void decreaseIndent() {
    if (indent_level_ >= 2) indent_level_ -= 2;
  }

  std::ostream& stream(bool indent = true) {
    if (indent) {
      static const char* spaces = "                                ";
      for (int i = 0; i < indent_level_ / 32; ++i) {
        stream_ << spaces;
      }
      stream_.write(spaces, indent_level_ % 32);
    }
    return stream_;
  }

  int expression_level_;
  int indent_level_;
  std::ostream& stream_;
};

} // namespace

namespace monicelli {

void printAst(std::ostream& stream, const AstNode* node) {
  AstPrinter printer{stream};
  printer.visit(node);
}

} // namespace monicelli
