# Copyright 2017 the Monicelli project authors. All rights reserved.
# Use of this source code is governed by a GPLv3 license, see LICENSE.txt.

find_program(RAGEL_EXECUTABLE ragel)

if (${RAGEL_EXECUTABLE} STREQUAL "RAGEL_EXECUTABLE-NOTFOUND")
  message(FATAL_ERROR "Please install ragel to compile Monicelli.")
else()
  message(STATUS "Found Ragel: ${RAGEL_EXECUTABLE}")
endif()

function(add_ragel_library name ragel_source)
  set(generated_source "${ragel_source}.cpp")
  set(extra_sources "${ARGN}")

  add_custom_command(
    OUTPUT "${generated_source}"
    MAIN_DEPENDENCY "${ragel_source}"
    COMMAND ${RAGEL_EXECUTABLE} -G2 "${CMAKE_CURRENT_SOURCE_DIR}/${ragel_source}" -o "${generated_source}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    VERBATIM
  )

  add_library(${name} "${generated_source}" "${extra_sources}")

  set_target_properties(${name}
    PROPERTIES
      CXX_STANDARD 11
      CXX_STANDARD_REQUIRED true
  )

  if (NOT MSVC)
    # The lexer uses implicit fallthroughs all over, but it's OK.
    target_compile_options(${name} PRIVATE -Wno-implicit-fallthrough)
  endif()

  target_include_directories(${name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
endfunction()
