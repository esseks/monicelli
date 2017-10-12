// Copyright 2017 the Monicelli project authors. All rights reserved.
// Use of this source code is governed by a GPLv3 license, see LICENSE.txt.

#ifndef MONICELLI_PARSER_H
#define MONICELLI_PARSER_H

#include "ast.h"
#include "errors.h"
#include "lexer.h"
#include "support.h"

#include <iostream>
#include <memory>
#include <vector>

namespace monicelli {

class Parser final : public ErrorReportingMixin {
public:
  Parser(std::istream& input, const std::string& source_filename)
      : ErrorReportingMixin(source_filename), lexer_{input} {}

  std::unique_ptr<Module> parse() {
    current_token_ = lexer_.getNextToken();
    return parseModule();
  }

  void setLexerTrace(bool enabled) { lexer_.setTraceEnabled(enabled); }

private:
  Variable parseVariable();
  VarType parseType();
  std::unique_ptr<Module> parseModule();
  std::unique_ptr<Function> parseFunction();
  std::unique_ptr<Function> parseEntryPoint();
  std::unique_ptr<Statement> parseStatement();
  std::vector<std::unique_ptr<Statement>> parseStatements();
  std::unique_ptr<Statement> maybeParseStatement();
  std::unique_ptr<AssertStatement> parseAssertStatement();
  std::unique_ptr<InputStatement> parseInputStatement();
  std::unique_ptr<AbortStatement> parseAbortStatement();
  BranchCase parseBranchCase(std::shared_ptr<Expression> condition_lhs);
  std::unique_ptr<BranchElse> parseBranchElse();
  std::unique_ptr<BranchStatement> parseBranchStatement();
  std::unique_ptr<VardeclStatement> parseVardeclStatement();
  std::unique_ptr<LoopStatement> parseLoopStatement();
  std::unique_ptr<ReturnStatement> parseReturnStatement();
  std::unique_ptr<Expression> parseExpression();
  std::unique_ptr<Expression> parseSemiExpression(std::shared_ptr<Expression> lhs);
  std::unique_ptr<Expression> maybeParseExpression() { return maybeParseExpressionInternal(0); }
  std::unique_ptr<Expression> maybeParseExpressionInternal(int min_precedence);
  std::unique_ptr<Expression> maybeParseAtomicExpression();
  std::unique_ptr<FunctionCallExpression> parseFunctionCallExpression();

  std::unique_ptr<Token> getNextToken();

  Token* peekNextToken() {
    assert(current_token_ && "Cannot peek into an empty stream.");
    return current_token_.get();
  }

  void ignoreNextToken() {
    auto token = getNextToken();
    USE(token);
  }

  Lexer lexer_;
  std::unique_ptr<Token> current_token_;
};

} // namespace monicelli

#endif
