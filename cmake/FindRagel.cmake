# Copyright 2017 the Monicelli project authors. All rights reserved.
# Use of this source code is governed by a GPLv3 license, see LICENSE.txt.

find_program(RAGEL ragel)

if (${RAGEL} STREQUAL "RAGEL-NOTFOUND")
  message(FATAL_ERROR "Please install ragel to compile Monicelli.")
else()
  message(STATUS "Found ragel: ${RAGEL}")
endif()
