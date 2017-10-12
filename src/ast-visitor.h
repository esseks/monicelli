#ifndef MONICELLI_AST_VISITOR_H
#define MONICELLI_AST_VISITOR_H

// Copyright 2017 the Monicelli project authors. All rights reserved.
// Use of this source code is governed by a GPLv3 license, see LICENSE.txt.

#include "ast.def"
#include "ast.h"
#include "errors.h"

namespace monicelli {

#define DECLARE_CLASS(NAME) class NAME;

#define DEFAULT_VISIT(NAME) \
  T visit##NAME(const NAME*) { UNREACHABLE("Unhandled " #NAME "."); }

#define DISPATCH_CONST_STATEMENT(NAME) \
  case AstNode::TYPE_##NAME: \
    return derived().visit##NAME(static_cast<const NAME*>(node));

AST_NODES(DECLARE_CLASS)

template<typename AstVisitorImpl, typename T> class ConstAstVisitor {
public:
  AST_NODES(DEFAULT_VISIT)

  T visit(const AstNode* node) {
    switch (node->getClassType()) {
      AST_NODES(DISPATCH_CONST_STATEMENT)
    default:
      UNREACHABLE("Unknown AstNode type class.");
    }
  }

  T visit(const AstNode& node) { return visit(&node); }

private:
  AstVisitorImpl& derived() { return *static_cast<AstVisitorImpl*>(this); }
};

#undef DECLARE_CLASS
#undef DEFAULT_VISIT
#undef DISPATCH_CONST_STATEMENT

} // namespace monicelli

#endif
