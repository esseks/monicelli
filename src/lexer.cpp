// Copyright 2017 the Monicelli project authors. All rights reserved.
// Use of this source code is governed by a GPLv3 license, see LICENSE.txt.

#include "lexer.h"
#include "errors.h"

#include <cstring>

namespace monicelli {

static const char* builtinTypeToString(Token::BuiltinTypeValue type) {
  switch (type) {
#define RETURN_TYPE_NAME(NAME, _1, _2, _3, _4, _5) \
  case Token::BUILTIN_TYPE_##NAME: \
    return #NAME;
    BUILTIN_TYPES(RETURN_TYPE_NAME)
#undef RETURN_TYPE_NAME
  default:
    UNREACHABLE("Unhandled BuiltinType.");
  }
}

void Token::print(std::ostream& stream) {
  switch (type_) {
#define PRINT_TOKEN_NAME(TOKEN, _) \
  case Token::TOKEN_##TOKEN: \
    stream << "<" #TOKEN; \
    break;
    LEXER_TOKENS(PRINT_TOKEN_NAME)
#undef PRINT_TOKEN_NAME
  }
  switch (getValueTypeForToken(type_)) {
  case ValueType::INTEGER:
    stream << '(' << int_value_ << ')';
    break;
  case ValueType::FLOAT:
    stream << '(' << fp_value_ << ')';
    break;
  case ValueType::STRING:
    stream << '(' << string_value_ << ')';
    break;
  case ValueType::BUILTIN_TYPE:
    stream << '(' << builtinTypeToString(builtin_type_value_) << ')';
    break;
  case ValueType::VOID:
  default:
    break;
  }
  stream << '@' << getFirstLocation() << '-' << getLastLocation() << ">\n";
}

bool Token::isOperator() const {
  switch (type_) {
#define CASE_NAME(NAME, _) case Token::TOKEN_##NAME:
    LEXER_OPERATOR_TOKENS(CASE_NAME)
#undef CASE_NAME
    return true;
  default:
    return false;
  }
}

// static
Token::ValueType Token::getValueTypeForToken(Token::TokenType type) {
  switch (type) {
#define RETURN_VALUE_TYPE(TYPE, VALUE_TYPE) \
  case Token::TOKEN_##TYPE: \
    return ValueType::VALUE_TYPE;
    LEXER_TOKENS(RETURN_VALUE_TYPE)
#undef RETURN_VALUE_TYPE
  default:
    UNREACHABLE("Unknown token type.");
  }
}

void Buffer::imbue(std::istream& input) {
  int to_read = capacity_ - size_;

  if (to_read <= 0) {
    // Grow buffer.
  }

  input.read(data_.get() + size_, to_read);
  size_ += input.gcount();
  cursor_ = data_.get();
}

void Lexer::advanceBuffer() {
  // If there is a match in progress, keep it.
  if (state_.ts) {
    int ts_offset = state_.ts - buffer_.getData();
    buffer_.shift(ts_offset);
    state_.ts = buffer_.getData();
    state_.te -= ts_offset;
  } else {
    buffer_.clear();
  }

  buffer_.imbue(input_);
}

} // namespace monicelli
