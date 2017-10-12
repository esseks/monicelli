# Copyright 2017 the Monicelli project authors. All rights reserved.
# Use of this source code is governed by a GPLv3 license, see LICENSE.txt.

find_program(LLVM_CONFIG llvm-config)

if (LLVM_CONFIG STREQUAL "LLVM_CONFIG-NOTFOUND")
  message(FATAL_ERROR "Please install the LLVM dev package to compile Monicelli.")
else()
  message(STATUS "Found llvm-config: ${LLVM_CONFIG}")
endif()

execute_process(
  COMMAND ${LLVM_CONFIG} --version
  OUTPUT_VARIABLE LLVM_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(TARGET_LLVM_VERSION "3.8.0")

if (NOT LLVM_VERSION STREQUAL ${TARGET_LLVM_VERSION})
  message(WARNING "Expected LLVM ${TARGET_LLVM_VERSION}, found ${LLVM_VERSION}, build may fail.")
endif()

execute_process(
  COMMAND ${LLVM_CONFIG} --includedir
  OUTPUT_VARIABLE LLVM_INCLUDE_DIR
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND ${LLVM_CONFIG} --libdir
  OUTPUT_VARIABLE LLVM_LIBRARY_DIR
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND ${LLVM_CONFIG} --cxxflags
  OUTPUT_VARIABLE LLVM_CXXFLAGS
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND ${LLVM_CONFIG} --libs all
  OUTPUT_VARIABLE LLVM_MODULE_LIBS
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND ${LLVM_CONFIG} --system-libs
  OUTPUT_VARIABLE LLVM_SYSTEM_LIBS
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(LLVM_LIBS ${LLVM_MODULE_LIBS} ${LLVM_SYSTEM_LIBS})
