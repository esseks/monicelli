// Copyright 2017 the Monicelli project authors. All rights reserved.
// Use of this source code is governed by a GPLv3 license, see LICENSE.txt.

#ifndef MONICELLI_OPTIONS_H
#define MONICELLI_OPTIONS_H

#include "iterators.h"

#include <string>
#include <vector>

namespace monicelli {

class ProgramOptions final {
public:
  typedef std::vector<std::string>::const_iterator ConstStringIter;

  static ProgramOptions fromCommandLine(int argc, char** argv);

  bool shouldPrintIR() const { return print_ir_; }
  bool shouldPrintAST() const { return print_ast_; }
  bool shouldTraceLexer() const { return trace_lexer_; }
  bool shouldOnlyCompile() const { return compile_only_; }
  bool shouldSkipCompilation() const { return skip_compile_; }
  const std::string& getOutputFilename() const { return output_filename_; }

  ConstStringIter begin_input_filenames() const { return input_filenames_.cbegin(); }
  ConstStringIter end_input_filenames() const { return input_filenames_.cend(); }
  ConstRangeWrapper<ConstStringIter> input_filenames() const {
    return {begin_input_filenames(), end_input_filenames()};
  }
  int input_filenames_size() const { return input_filenames_.size(); }
  bool input_filenames_empty() const { return input_filenames_.empty(); }

  const std::string& getCPU() const { return cpu_; }
  const std::string& getCPUFeatures() const { return cpu_features_; }
  bool shouldEmitPIC() const { return emit_pic_; }

private:
  static void printHelp(const char* program_name);

  ProgramOptions()
      : print_ir_(false), print_ast_(false), trace_lexer_(false), compile_only_(false),
        skip_compile_(false), cpu_("generic"), emit_pic_(true) {}

  bool print_ir_;
  bool print_ast_;
  bool trace_lexer_;
  bool compile_only_;
  bool skip_compile_;
  std::vector<std::string> input_filenames_;
  std::string output_filename_;
  std::string cpu_;
  std::string cpu_features_;
  bool emit_pic_;
};

} // namespace monicelli

#endif
