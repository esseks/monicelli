#ifndef MONICELLI_CODEGEN_H
#define MONICELLI_CODEGEN_H

// Copyright 2017 the Monicelli project authors. All rights reserved.
// Use of this source code is governed by a GPLv3 license, see LICENSE.txt.

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include <memory>

namespace monicelli {

class Module;

std::unique_ptr<llvm::Module> generateIR(llvm::LLVMContext& context, Module* ast);

void runFunctionOptimizer(llvm::Module* module);

void printIR(std::ostream& stream, llvm::Module* module);

} // namespace monicelli

#endif
