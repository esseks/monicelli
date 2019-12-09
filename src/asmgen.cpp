// Copyright 2017 the Monicelli project authors. All rights reserved.
// Use of this source code is governed by a GPLv3 license, see LICENSE.txt.

#include "asmgen.h"
#include "errors.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"

#include <cstdlib>
#include <iostream>
#include <string>

#ifdef MONICELLI_ENABLE_LINKER
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace monicelli {

void registerTargets() {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmParser();
  llvm::InitializeNativeTargetAsmPrinter();
}

llvm::TargetMachine* getTargetMachine(const std::string& triple, const std::string& cpu,
                                      const std::string& features, bool emit_pic) {
  std::string error;
  auto target = llvm::TargetRegistry::lookupTarget(triple, error);

  if (!target) {
    std::cerr << "While determining target: " << error << '\n';
    exit(1);
  }

  llvm::TargetOptions opt;
  auto reloc_model = emit_pic ? llvm::Reloc::Model::PIC_ : llvm::Reloc::Model::Static;
  return target->createTargetMachine(triple, cpu, features, opt, reloc_model);
}

void writeAssembly(const std::string& to_filename, llvm::Module* module,
                   llvm::TargetMachine* target_machine) {
  std::error_code error_code;
  llvm::raw_fd_ostream output{to_filename, error_code, llvm::sys::fs::F_None};

  if (error_code) {
    std::cerr << "Could not open '" << to_filename << "' for output: " << error_code.message()
              << '\n';
    exit(1);
  }

  llvm::legacy::PassManager asm_generator;
  auto file_type = llvm::TargetMachine::CGFT_ObjectFile;

  if (target_machine->addPassesToEmitFile(asm_generator, output, nullptr, file_type)) {
    std::cerr << "Cannot emit an object file of this type\n";
    exit(1);
  }

  asm_generator.run(*module);
  output.flush();
}

#ifdef MONICELLI_ENABLE_LINKER

static const char* C_COMPILER = "c99";

void linkAssembly(const std::string& output_name, const std::vector<std::string>& object_files,
                  bool keep_object_files) {
  // Linking a C object file with certain modern libc's is so complicated that
  // we just let a C compiler do it for us. This function assumes POSIX, and
  // most recent POSIX-compliant systems will also adopt the recommendation
  // to have a C compiler installed and called c99. Very old systems will have
  // c89 instead. cc exists as well, but it's not specified by POSIX.

  int cc_argc = object_files.size() + 1 + 2 + 1;
  std::unique_ptr<const char* []> cc_args { new const char*[cc_argc] };
  int i = 0;
  cc_args[i++] = C_COMPILER;
  if (!output_name.empty()) {
    cc_args[i++] = "-o";
    cc_args[i++] = output_name.c_str();
  }
  for (const auto& object_file : object_files) {
    assert(object_file[0] != '-' && "The option parser allowed a filename starting with -");
    cc_args[i++] = object_file.c_str();
  }
  cc_args[i] = nullptr;

  pid_t pid = fork();

  if (pid == 0) {
    if (execvp(C_COMPILER, const_cast<char* const*>(cc_args.get())) == -1) {
      std::cerr << "Failed to launch the linker. Check that '" << C_COMPILER << "' is installed.\n";
      exit(1);
    }
    UNREACHABLE("Successfully returned from exec()?");
  }

  if (pid == -1) {
    std::cerr << "Failed to spawn the linker process.\n";
    exit(1);
  }

  waitpid(pid, nullptr, 0);

  if (keep_object_files) return;

  for (const auto& object_file : object_files) {
    unlink(object_file.c_str());
  }
}

#endif

} // namespace monicelli
