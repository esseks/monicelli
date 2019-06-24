# Copyright 2017 the Monicelli project authors. All rights reserved.
# Use of this source code is governed by a GPLv3 license, see LICENSE.txt.

find_program(RAGEL ragel)

if (${RAGEL} STREQUAL "RAGEL-NOTFOUND")
  message(FATAL_ERROR "Please install ragel to compile Monicelli.")
else()
  message(STATUS "Found ragel: ${RAGEL}")
endif()

function(add_ragel_library name source header)
  set(generated_cpp "${source}.cpp")

  add_custom_command(
    OUTPUT "${generated_cpp}"
    MAIN_DEPENDENCY "${source}"
    DEPENDS "${header}"
    COMMAND ${RAGEL} -G2 "${CMAKE_CURRENT_SOURCE_DIR}/${source}" -o "${generated_cpp}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    VERBATIM
  )

  add_library(${name} "${generated_cpp}")

  set_target_properties(${name}
    PROPERTIES
      CXX_STANDARD 11
      CXX_STANDARD_REQUIRED true
  )

  # The lexer uses implicit fallthroughs all over, but it's OK.
  target_compile_options(${name} PRIVATE -Wno-implicit-fallthrough)

  target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
endfunction()
