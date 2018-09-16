// Copyright 2017 the Monicelli project authors. All rights reserved.
// Use of this source code is governed by a GPLv3 license, see LICENSE.txt.

#include "asmgen.h"
#include "ast-printer.h"
#include "codegen.h"
#include "options.h"
#include "parser.h"

#include <fstream>
#include <string>
#include <vector>

using namespace monicelli;

int main(int argc, char** argv) {
  ProgramOptions options = ProgramOptions::fromCommandLine(argc, argv);
  if (options.input_filenames_empty()) {
    std::cerr << "No input files.\n";
    return 0;
  }

  if (options.shouldOnlyCompile() && options.input_filenames_size() > 1 &&
      !options.getOutputFilename().empty()) {
    std::cerr << "Output filename in compile mode may be specified only with a "
                 "single input file.\n";
    return 1;
  }

  registerTargets();

  auto triple = llvm::sys::getDefaultTargetTriple();
  auto target_machine =
      getTargetMachine(triple, options.getCPU(), options.getCPUFeatures(), options.shouldEmitPIC());

#ifdef MONICELLI_ENABLE_LINKER
  std::vector<std::string> object_filenames;
  object_filenames.reserve(options.input_filenames_size());
#endif

  for (const auto& input_filename : options.input_filenames()) {
    std::ifstream input{input_filename};
    if (!input) {
      std::cerr << "Cannot open input file " << input_filename << ".\n";
      return 1;
    }

    Parser parser{input, input_filename};
    parser.setLexerTrace(options.shouldTraceLexer());
    auto ast = parser.parse();

    if (options.shouldPrintAST()) {
      printAst(std::cout, ast.get());
      continue;
    }

    llvm::LLVMContext context;
    auto ir = generateIR(context, ast.get());
    ir->setTargetTriple(triple);
    ir->setDataLayout(target_machine->createDataLayout());
    runFunctionOptimizer(ir.get());

    if (options.shouldPrintIR()) {
      printIR(std::cout, ir.get());
      continue;
    }

    if (options.shouldSkipCompilation()) continue;

    std::string object_filename;
    if (options.shouldOnlyCompile() && !options.getOutputFilename().empty()) {
      object_filename = options.getOutputFilename();
    } else {
      object_filename = basename(input_filename) + ".o";
    }

    writeAssembly(object_filename, ir.get(), target_machine);

#ifdef MONICELLI_ENABLE_LINKER
    object_filenames.emplace_back(std::move(object_filename));
#endif
  }

#ifdef MONICELLI_ENABLE_LINKER
  if (!options.shouldSkipCompilation() && !options.shouldOnlyCompile() &&
      !object_filenames.empty()) {
    linkAssembly(options.getOutputFilename(), object_filenames);
  }
#endif
}
