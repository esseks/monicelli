# Copyright 2017 the Monicelli project authors. All rights reserved.
# Use of this source code is governed by a GPLv3 license, see LICENSE.txt.

find_package(LLVM REQUIRED CONFIG)
find_package(Ragel REQUIRED)

option(MONICELLI_ENABLE_LINKER "Enable the Monicelli linker. Requires POSIX." ON)

add_ragel_library(lexer lexer.rl lexer.h)

add_executable(mcc
  main.cpp
  asmgen.cpp
  codegen.cpp
  codegen.def
  ast.cpp
  ast.def
  ast-visitor.h
  ast-printer.cpp
  parser.cpp
  lexer.cpp
  lexer.def
  options.cpp
  errors.cpp
  support.cpp
  location.h
  iterators.h
  types.def
  operators.def
)

set_target_properties(mcc
  PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED true
)

target_compile_options(mcc PRIVATE -Wall -Wextra -Werror)

if (MONICELLI_ENABLE_LINKER)
  target_compile_definitions(mcc PRIVATE MONICELLI_ENABLE_LINKER)
endif()

# TODO expand this line to support compiling on other architectures and cross-compilation.
llvm_config(mcc core support x86codegen x86asmparser)
target_link_libraries(mcc PRIVATE lexer)

install(TARGETS mcc RUNTIME DESTINATION bin)
