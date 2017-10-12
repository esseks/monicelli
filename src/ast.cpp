// Copyright 2017 the Monicelli project authors. All rights reserved.
// Use of this source code is governed by a GPLv3 license, see LICENSE.txt.

#include "ast.h"
#include "errors.h"

namespace monicelli {

// static
const char* BinaryExpression::getOperatorRepresentation(BinaryExpression::Type type) {
  switch (type) {
#define RETURN_OP_STRING(_, NAME, __, STRING) \
  case Type::NAME: \
    return STRING;
    AST_BINARY_OPERATORS(RETURN_OP_STRING)
#undef RETURN_OP_STRING
  default:
    UNREACHABLE("Unhandled BinaryExpression type.");
  }
}

} // namespace monicelli
