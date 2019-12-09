# Copyright 2017 the Monicelli project authors. All rights reserved.
# Use of this source code is governed by a GPLv3 license, see LICENSE.txt.

cmake_minimum_required(VERSION 3.7)
project(Monicelli VERSION 2.0.0 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(CPACK_GENERATOR TGZ)
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})

include(CPack)
include(Doxycelli)

if (MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra -pedantic -Werror -Wno-unused-parameter)
endif()

add_subdirectory(src)

install(FILES README.md LICENSE.txt DESTINATION doc/monicelli)
