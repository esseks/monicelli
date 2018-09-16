#ifndef MONICELLI_ASMGEN_H
#define MONICELLI_ASMGEN_H

// Copyright 2017 the Monicelli project authors. All rights reserved.
// Use of this source code is governed by a GPLv3 license, see LICENSE.txt.

#include "llvm/IR/Module.h"
#include "llvm/Target/TargetMachine.h"

#include <string>
#include <vector>

namespace monicelli {

void registerTargets();

llvm::TargetMachine* getTargetMachine(const std::string& triple, const std::string& cpu,
                                      const std::string& features, bool emit_pic);

void writeAssembly(const std::string& to_filename, llvm::Module* module,
                   llvm::TargetMachine* target_machine);

#ifdef MONICELLI_ENABLE_LINKER
void linkAssembly(const std::string& output_name, const std::vector<std::string>& object_files,
                  bool keep_object_files = false);
#endif

} // namespace monicelli

#endif
