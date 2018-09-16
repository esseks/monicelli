// Copyright 2017 the Monicelli project authors. All rights reserved.
// Use of this source code is governed by a GPLv3 license, see LICENSE.txt.

#include "options.h"

#include <cstdlib>
#include <cstring>
#include <iostream>

namespace monicelli {

// static
ProgramOptions ProgramOptions::fromCommandLine(int argc, char** argv) {
  ProgramOptions options;
  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
      if (i == argc - 1) {
        std::cerr << "--output must be followed by a filename.\n";
        break;
      }
      options.output_filename_ = argv[++i];
      continue;
    }
    if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--print-ir") == 0) {
      options.print_ir_ = true;
      continue;
    }
    if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--print-ast") == 0) {
      options.print_ast_ = true;
      continue;
    }
    if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--trace-lexer") == 0) {
      options.trace_lexer_ = true;
      continue;
    }
    if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--no-compile") == 0) {
      options.skip_compile_ = true;
      continue;
    }
    if (strcmp(argv[i], "--no-pic") == 0) {
      options.emit_pic_ = false;
      continue;
    }
#ifdef MONICELLI_ENABLE_LINKER
    if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--only-compile") == 0) {
      options.compile_only_ = true;
      continue;
    }
#endif
    if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--cpu") == 0) {
      if (i == argc - 1) {
        std::cerr << "--cpu must be followed by a CPU name.\n";
        break;
      }
      options.cpu_ = argv[++i];
    }
    if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--cpu-features") == 0) {
      if (i == argc - 1) {
        std::cerr << "--cpu-features must be followed by a set of features.\n";
        break;
      }
      options.cpu_features_ = argv[++i];
    }
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      printHelp(argv[0]);
    }
    if (argv[i][0] == '-') {
      std::cerr << "Unknown option " << argv[i] << ".\n\n";
      printHelp(argv[0]);
      exit(1);
    }
    options.input_filenames_.emplace_back(argv[i]);
  }
#ifndef MONICELLI_ENABLE_LINKER
  options.compile_only_ = true;
#endif
  return options;
}

// static
void ProgramOptions::printHelp(const char* program_name) {
  std::cout << "Usage: " << program_name
            << " [options...] [input.mc ...]\n\n"
               "Options:\n"
#ifdef MONICELLI_ENABLE_LINKER
               "  --only-compile, -c      : Compile only, do not link.\n"
#endif
               "  --no-compile, -n        : Do not compile, only print (see below).\n"
               "  --output, -o out.o      : Specify the output filename.\n"
               "  --trace-lexer, -t       : Print tokens as seen by the lexer.\n"
               "  --print-ast, -p         : Print the AST as pseudocode.\n"
               "  --print-ir, -s          : Print the IR of the code.\n"
               "  --cpu, -m model         : Set the CPU model to this (default: generic).\n"
               "  --cpu-features, -f feat : Enable these CPU features (default: none).\n"
               "  --no-pic                : Disable position independent code.\n"
               "  --help, -h              : Print this message.\n"
               "\n";
  exit(0);
}

} // namespace monicelli
