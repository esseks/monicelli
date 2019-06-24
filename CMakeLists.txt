# Copyright 2017 the Monicelli project authors. All rights reserved.
# Use of this source code is governed by a GPLv3 license, see LICENSE.txt.

cmake_minimum_required(VERSION 3.5)
project(Monicelli CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(MonicelliDoc)

add_subdirectory(src)

install(FILES README.md LICENSE.txt DESTINATION doc/monicelli)
