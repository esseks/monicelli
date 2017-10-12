// Copyright 2017 the Monicelli project authors. All rights reserved.
// Use of this source code is governed by a GPLv3 license, see LICENSE.txt.

#include "parser.h"
#include "errors.h"

namespace monicelli {

std::unique_ptr<Module> Parser::parseModule() {
  std::unique_ptr<Module> module{new Module};

  while (peekNextToken()->getType() == Token::TOKEN_FUN_DECL) {
    module->functions_.emplace_back(parseFunction());
  }

  if (peekNextToken()->getType() == Token::TOKEN_ENTRY_POINT) {
    module->maybe_entry_point_ = parseEntryPoint();
  }

  while (peekNextToken()->getType() == Token::TOKEN_FUN_DECL) {
    module->functions_.emplace_back(parseFunction());
  }

  auto token = getNextToken();
  if (token->getType() != Token::TOKEN_END) {
    error(token, "expected end of file");
  }

  module->source_filename_ = getSourceFilename();

  return module;
}

std::unique_ptr<Function> Parser::parseEntryPoint() {
  std::unique_ptr<Function> function{new Function};

  auto token = getNextToken();
  if (token->getType() != Token::TOKEN_ENTRY_POINT) {
    error(token, "expected entry point declaration");
  }

  function->return_type_.base_type_ = VarType::INTEGER;
  function->body_ = parseStatements();

  return function;
}

std::unique_ptr<Function> Parser::parseFunction() {
  std::unique_ptr<Function> function{new Function};

  auto token = getNextToken();
  if (token->getType() != Token::TOKEN_FUN_DECL) {
    error(token, "expected function declaration");
  }

  switch (peekNextToken()->getType()) {
  case Token::TOKEN_STAR:
  case Token::TOKEN_TYPENAME:
    function->return_type_ = parseType();
    break;
  default:
    break;
  }

  token = getNextToken();
  if (token->getType() != Token::TOKEN_IDENTIFIER) {
    error(token, "expected function name");
  }
  function->name_ = token->getStringValue();

  token = getNextToken();
  switch (token->getType()) {
  case Token::TOKEN_FUN_PARAMS:
    for (bool done = false; !done;) {
      auto var = parseVariable();
      auto type = parseType();
      function->params_.emplace_back(var, type);
      auto token = getNextToken();
      switch (token->getType()) {
      case Token::TOKEN_COMMA:
        break;
      case Token::TOKEN_FUN_END:
        done = true;
        break;
      default:
        error(token, "expected either more parameters or function body begin");
        break;
      }
    }
    // fallthrough
  case Token::TOKEN_FUN_END:
    break;
  default:
    error(token, "expected either parameters or function body begin");
    break;
  }

  function->body_ = parseStatements();

  return function;
}

Variable Parser::parseVariable() {
  auto token = getNextToken();
  if (token->getType() == Token::TOKEN_ARTICLE) {
    token = getNextToken();
  }
  if (token->getType() != Token::TOKEN_IDENTIFIER) {
    error(token, "expected variable name");
  }

  Variable var;
  var.name_ = token->getStringValue();
  var.first_location_ = token->getFirstLocation();
  var.last_location_ = token->getLastLocation();
  return var;
}

static VarType::BaseType builtinTypeToASTType(Token::BuiltinTypeValue type) {
  switch (type) {
#define RETURN_VAR_TYPE(NAME, _1, _2, _3, _4, _5) \
  case Token::BUILTIN_TYPE_##NAME: \
    return VarType::NAME;
    BUILTIN_TYPES(RETURN_VAR_TYPE)
  default:
    UNREACHABLE("Unhandled BuiltinType.");
  }
}

VarType Parser::parseType() {
  VarType type;
  auto token = getNextToken();
  if (token->getType() == Token::TOKEN_STAR) {
    type.pointer_ = true;
    token = getNextToken();
  }
  if (token->getType() != Token::TOKEN_TYPENAME) {
    error(token, "expected type name");
  }
  type.base_type_ = builtinTypeToASTType(token->getBuiltinTypeValue());
  return type;
}

std::vector<std::unique_ptr<Statement>> Parser::parseStatements() {
  std::vector<std::unique_ptr<Statement>> statements;
  while (true) {
    auto statement = maybeParseStatement();
    if (!statement) break;
    statements.emplace_back(std::move(statement));
  }
  return statements;
}

std::unique_ptr<Statement> Parser::parseStatement() {
  auto start_location = peekNextToken()->getFirstLocation();
  auto statement = maybeParseStatement();
  if (!statement) {
    error(start_location, "expected statement");
  }
  return statement;
}

std::unique_ptr<Statement> Parser::maybeParseStatement() {
  switch (peekNextToken()->getType()) {
  case Token::TOKEN_ASSERT:
    return parseAssertStatement();
  case Token::TOKEN_INPUT:
    return parseInputStatement();
  case Token::TOKEN_ABORT:
    return parseAbortStatement();
  case Token::TOKEN_BRANCH_CONDITION:
    return parseBranchStatement();
  case Token::TOKEN_VARDECL:
    return parseVardeclStatement();
  case Token::TOKEN_LOOP_BEGIN:
    return parseLoopStatement();
  case Token::TOKEN_RETURN:
    return parseReturnStatement();
  case Token::TOKEN_COMMA:
    ignoreNextToken();
    return maybeParseStatement();
  default:
    break;
  }

  // If we are here, the statement starts with an expression.
  auto expression = maybeParseExpression();

  // If there was not an expression here, then it's not a statement.
  if (!expression) return nullptr;

  auto token = peekNextToken();
  switch (token->getType()) {
  case Token::TOKEN_PRINT: {
    ignoreNextToken();
    std::unique_ptr<PrintStatement> statement{new PrintStatement};
    statement->expression_ = std::move(expression);
    return statement;
  }
  case Token::TOKEN_ASSIGN: {
    AtomicExpression* e = dynamic_cast<AtomicExpression*>(expression.get());
    if (!e || e->getType() != AtomicExpression::IDENTIFIER) {
      error(token, "assignment target must be an identifier");
    }
    ignoreNextToken();
    std::unique_ptr<AssignStatement> statement{new AssignStatement};
    statement->variable_ = e->getIdentifierValue();
    statement->expression_ = parseExpression();
    return statement;
  }
  default:
    if (expression->isFunctionCall()) {
      std::unique_ptr<ExpressionStatement> statement{new ExpressionStatement};
      statement->expression_ = std::move(expression);
      return statement;
    }
    error(token, "only a function call can be a statement");
    break;
  }

  UNREACHABLE("Unhandled statement type in parser");
}

std::unique_ptr<AssertStatement> Parser::parseAssertStatement() {
  auto token = getNextToken();
  if (token->getType() != Token::TOKEN_ASSERT) {
    error(token, "expected assert statement");
  }
  std::unique_ptr<AssertStatement> statement{new AssertStatement};
  statement->expression_ = parseExpression();
  token = getNextToken();
  if (token->getType() != Token::TOKEN_BANG) {
    error(token, "expected final !");
  }
  return statement;
}

std::unique_ptr<FunctionCallExpression> Parser::parseFunctionCallExpression() {
  auto token = getNextToken();
  if (token->getType() != Token::TOKEN_FUN_CALL) {
    error(token, "expected function call");
  }

  std::unique_ptr<FunctionCallExpression> statement{new FunctionCallExpression};
  statement->first_location_ = token->first_location_;

  token = getNextToken();
  if (token->getType() != Token::TOKEN_IDENTIFIER) {
    error(token, "expected name of the function to call");
  }

  statement->function_name_ = token->getStringValue();

  token = getNextToken();
  switch (token->getType()) {
  case Token::TOKEN_FUN_PARAMS:
    for (bool done = false; !done;) {
      statement->function_args_.emplace_back(parseExpression());
      auto token = getNextToken();
      switch (token->getType()) {
      case Token::TOKEN_FUN_END:
        done = true;
        break;
      case Token::TOKEN_COMMA:
        break;
      default:
        error(token, "expected either more params or end of call statement");
        break;
      }
    }
    // fallthrough
  case Token::TOKEN_FUN_END:
    break;
  default:
    error(token, "expected either call params or end of call statement");
    break;
  }

  statement->last_location_ = peekNextToken()->first_location_;
  return statement;
}

std::unique_ptr<InputStatement> Parser::parseInputStatement() {
  auto token = getNextToken();
  if (token->getType() != Token::TOKEN_INPUT) {
    error(token, "expected input statement");
  }
  std::unique_ptr<InputStatement> statement{new InputStatement};
  statement->variable_ = parseVariable();
  return statement;
}

std::unique_ptr<AbortStatement> Parser::parseAbortStatement() {
  auto token = getNextToken();
  if (token->getType() != Token::TOKEN_ABORT) {
    error(token, "expected abort statement");
  }
  return std::unique_ptr<AbortStatement>{new AbortStatement};
}

BranchCase Parser::parseBranchCase(std::shared_ptr<Expression> condition_lhs) {
  BranchCase branch_case;
  branch_case.expression_ = parseSemiExpression(condition_lhs);

  if (peekNextToken()->getType() == Token::TOKEN_COLON) {
    ignoreNextToken();
  }

  branch_case.body_ = parseStatements();

  return branch_case;
}

std::unique_ptr<BranchElse> Parser::parseBranchElse() {
  std::unique_ptr<BranchElse> else_case{new BranchElse};
  else_case->body_ = parseStatements();
  return else_case;
}

std::unique_ptr<BranchStatement> Parser::parseBranchStatement() {
  auto token = getNextToken();
  if (token->getType() != Token::TOKEN_BRANCH_CONDITION) {
    error(token, "expected branch condition");
  }

  std::unique_ptr<BranchStatement> statement{new BranchStatement};
  statement->lead_var_ = parseVariable();

  token = getNextToken();
  if (token->getType() != Token::TOKEN_BRANCH_BEGIN) {
    error(token, "expected begin of branch");
  }

  std::shared_ptr<Expression> condition_lhs{
      AtomicExpression::fromIdentifier(statement->lead_var_).release()};

  statement->cases_.emplace_back(parseBranchCase(condition_lhs));
  for (bool done = false; !done;) {
    switch (peekNextToken()->getType()) {
    case Token::TOKEN_CASE_END:
      ignoreNextToken();
      statement->cases_.emplace_back(parseBranchCase(condition_lhs));
      break;
    case Token::TOKEN_BRANCH_ELSE:
    case Token::TOKEN_BRANCH_END:
      done = true;
      break;
    default:
      error(peekNextToken(), "expected other cases, else case or end of branch");
      break;
    }
  }

  token = getNextToken();
  switch (token->getType()) {
  case Token::TOKEN_BRANCH_ELSE: {
    if (peekNextToken()->getType() == Token::TOKEN_COLON) {
      ignoreNextToken();
    }
    statement->maybe_else_case_ = parseBranchElse();
    auto token = getNextToken();
    if (token->getType() != Token::TOKEN_BRANCH_END) {
      error(token, "expected end of branch");
    }
    // fallthrough
  }
  case Token::TOKEN_BRANCH_END:
    break;
  default:
    error(token, "expected either else case or end of branch");
    break;
  }

  return statement;
}

std::unique_ptr<VardeclStatement> Parser::parseVardeclStatement() {
  auto token = getNextToken();
  if (token->getType() != Token::TOKEN_VARDECL) {
    error(token, "expected declaration");
  }

  std::unique_ptr<VardeclStatement> statement{new VardeclStatement};
  statement->variable_ = parseVariable();
  token = getNextToken();
  if (token->getType() != Token::TOKEN_COMMA) {
    error(token, "expected ,");
  }
  statement->type_ = parseType();

  if (peekNextToken()->getType() == Token::TOKEN_ASSIGN) {
    ignoreNextToken();
    statement->maybe_init_ = parseExpression();
  }

  return statement;
}

std::unique_ptr<LoopStatement> Parser::parseLoopStatement() {
  auto token = getNextToken();
  if (token->getType() != Token::TOKEN_LOOP_BEGIN) {
    error(token, "expected loop statement");
  }

  std::unique_ptr<LoopStatement> statement{new LoopStatement};
  while (peekNextToken()->getType() != Token::TOKEN_LOOP_CONDITION) {
    statement->body_.emplace_back(parseStatement());
  }
  ignoreNextToken(); // This was a Token::TOKEN_LOOP_CONDITION.

  statement->condition_ = parseExpression();

  return statement;
}

std::unique_ptr<ReturnStatement> Parser::parseReturnStatement() {
  auto token = getNextToken();
  if (token->getType() != Token::TOKEN_RETURN) {
    error(token, "expected return statement");
  }

  std::unique_ptr<ReturnStatement> statement{new ReturnStatement};
  if (peekNextToken()->getType() == Token::TOKEN_BANG) {
    ignoreNextToken();
    return statement;
  }
  statement->maybe_expression_ = parseExpression();

  token = getNextToken();
  if (token->getType() != Token::TOKEN_BANG) {
    error(token, "expected !");
  }

  return statement;
}

std::unique_ptr<Expression> Parser::parseExpression() {
  auto first_location = peekNextToken()->getFirstLocation();
  auto expression = maybeParseExpression();
  if (!expression) {
    error(first_location, "expected expression");
  }
  return expression;
}

static BinaryExpression::Type getOperatorTypeFromToken(const Token* token) {
  switch (token->getType()) {
#define TOKEN_OP_TO_EXPR_OP(TOKEN_NAME, EXPR_NAME, _, __) \
  case Token::TOKEN_##TOKEN_NAME: \
    return BinaryExpression::EXPR_NAME;
    AST_BINARY_OPERATORS(TOKEN_OP_TO_EXPR_OP)
#undef TOKEN_OP_TO_EXPR_OP
  default:
    UNREACHABLE("Unhandled token in operator conversion");
  }
}

std::unique_ptr<Expression> Parser::parseSemiExpression(std::shared_ptr<Expression> lhs) {
  BinaryExpression::Type op;
  if (peekNextToken()->isOperator()) {
    op = getOperatorTypeFromToken(getNextToken().get());
  } else {
    op = BinaryExpression::EQ;
  }

  auto rhs = parseExpression();

  return std::unique_ptr<Expression>{new BinaryExpression{op, lhs, rhs.release(), true}};
}

static int getOperatorPrecedenceFromToken(Token* token) {
  switch (token->getType()) {
#define RETURN_OP_PRIORITY(NAME, _, PRIORITY, __) \
  case Token::TOKEN_##NAME: \
    return PRIORITY;
    AST_BINARY_OPERATORS(RETURN_OP_PRIORITY)
#undef RETURN_OP_PRIORITY
  default:
    UNREACHABLE("Undefined operator priority for token");
  }
}

std::unique_ptr<Expression> Parser::maybeParseExpressionInternal(int min_precedence) {
  Location first_location = peekNextToken()->getFirstLocation();

  // Precedence climbing.
  auto lhs = maybeParseAtomicExpression();
  if (!lhs) return nullptr;

  while (true) {
    auto token = peekNextToken();
    if (!token->isOperator()) break;
    int precedence = getOperatorPrecedenceFromToken(token);
    if (precedence < min_precedence) break;
    auto op_type = getOperatorTypeFromToken(token);
    auto op_location = token->getFirstLocation();
    ignoreNextToken();
    auto rhs = maybeParseExpressionInternal(precedence + 1);
    if (!rhs) {
      error(op_location, "binary operation is missing a right side");
    }
    lhs.reset(new BinaryExpression{op_type, std::move(lhs), rhs.release(), false});
  }

  lhs->first_location_ = first_location;
  lhs->last_location_ = peekNextToken()->getFirstLocation();

  return lhs;
}

std::unique_ptr<Expression> Parser::maybeParseAtomicExpression() {
  switch (peekNextToken()->getType()) {
  case Token::TOKEN_ARTICLE:
  case Token::TOKEN_IDENTIFIER:
    return AtomicExpression::fromIdentifier(parseVariable());
  case Token::TOKEN_INTEGER:
    return AtomicExpression ::fromInt(getNextToken()->getIntValue());
  case Token::TOKEN_FLOAT:
    return AtomicExpression ::fromFloat(getNextToken()->getFloatValue());
  case Token::TOKEN_FUN_CALL:
    return parseFunctionCallExpression();
  default:
    return nullptr;
  }
}

std::unique_ptr<Token> Parser::getNextToken() {
  assert(current_token_ && "Cannot get from an empty stream");
  auto token = std::move(current_token_);
  switch (token->getType()) {
  case Token::TOKEN_END:
  case Token::TOKEN_UNKNOWN:
    current_token_ = nullptr;
    break;
  default:
    current_token_ = lexer_.getNextToken();
    break;
  }
  return token;
}

} // namespace monicelli
