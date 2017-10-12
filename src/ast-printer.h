#ifndef MONICELLI_AST_PRINTER_H
#define MONICELLI_AST_PRINTER_H

// Copyright 2017 the Monicelli project authors. All rights reserved.
// Use of this source code is governed by a GPLv3 license, see LICENSE.txt.

#include <iostream>

namespace monicelli {

class AstNode;

void printAst(std::ostream& stream, const AstNode* node);

} // namespace monicelli

#endif
