# Copyright 2017 the Monicelli project authors. All rights reserved.
# Use of this source code is governed by a GPLv3 license, see LICENSE.txt.

set(MONICELLI_ARCH "x86" CACHE STRING "Target architecture for Monicelli.")
set(MONICELLI_LLVM_VERSION 7 CACHE STRING "LLVM version for Monicelli.")
set(MONICELLI_LINKER ON CACHE BOOL "Enable the Monicelli linker. Requires POSIX.")

find_package(LLVM "${MONICELLI_LLVM_VERSION}" REQUIRED CONFIG)
find_package(Ragel REQUIRED)

add_ragel_library(lexer
  lexer.rl
  lexer.h
  lexer.cpp
  lexer.def
)

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
  options.cpp
  errors.cpp
  support.cpp
  location.h
  iterators.h
  types.def
  operators.def
)

set_target_properties(mcc lexer
  PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED true
)

target_include_directories(mcc PRIVATE ${LLVM_INCLUDE_DIRS})

target_compile_definitions(mcc PRIVATE ${LLVM_DEFINITIONS})

target_link_libraries(mcc PRIVATE lexer)

llvm_config(mcc
  core
  support
  "${MONICELLI_ARCH}codegen"
  "${MONICELLI_ARCH}asmparser"
)

if (MONICELLI_LINKER)
  target_compile_definitions(mcc PRIVATE MONICELLI_ENABLE_LINKER)
endif()

install(TARGETS mcc RUNTIME DESTINATION bin)
