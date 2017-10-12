# Copyright 2017 the Monicelli project authors. All rights reserved.
# Use of this source code is governed by a GPLv3 license, see LICENSE.txt.

find_package(LLVM REQUIRED)
find_package(Ragel REQUIRED)

option(ENABLE_LINKER "Enable the Monicelli linker. Requires POSIX." ON)

if (ENABLE_LINKER)
  add_definitions(-DMONICELLI_ENABLE_LINKER)
endif()

add_definitions(
  -std=c++11
  -Wall -Wextra -Werror
  # The lexer uses implicit fallthroughs all over, but it's OK.
  -Wno-implicit-fallthrough
  ${LLVM_CXXFLAGS}
  -g -O2 -UNDEBUG
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${LLVM_INCLUDE_DIR}
)

link_directories(
  ${LLVM_LIBRARY_DIR}
)

add_custom_command(
  OUTPUT lexer.rl.cpp
  MAIN_DEPENDENCY lexer.rl
  DEPENDS lexer.h
  COMMAND ${RAGEL} -G2 "${CMAKE_CURRENT_SOURCE_DIR}/lexer.rl" -o lexer.rl.cpp
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  VERBATIM
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
  lexer.cpp
  lexer.def
  "${CMAKE_CURRENT_BINARY_DIR}/lexer.rl.cpp"
  options.cpp
  errors.cpp
  support.cpp
  location.h
  iterators.h
  types.def
  operators.def
)

target_link_libraries(mcc ${LLVM_LIBS})

install(TARGETS mcc RUNTIME DESTINATION bin)
