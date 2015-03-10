#
# Monicelli: an esoteric language compiler
# 
# Copyright (C) 2014 Stefano Sanfilippo
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

project(Monicelli)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_BUILD_TYPE Release)

message("== Only a limited set of platforms was tested. We need your help!")
message("== Report build errors to https://github.com/esseks/monicelli/issues")

## 1. Compiler sanity check

try_run(
    execution_results
    supported_compiler
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/features.cpp
    COMPILE_DEFINITIONS -std=c++0x
    OUTPUT_VARIABLE features_build_log
)

if (checkfeat)
    message(${features_build_log})
endif()

if (NOT supported_compiler)
    message(FATAL_ERROR
        "Some C++11 features we need are not implemented by your compiler.\n"
        "Run cmake with -Dcheckfeat=1 to see the exact cause."
    )
endif()

if (execution_results MATCHES FAILED_TO_RUN)
    message(FATAL_ERROR
        "Your compiler supports the set of C++11 features we need, "
        "but something failed.\n"
        "Run cmake with -Dcheckfeat=1 to see the exact cause."
    )
endif()

## 2. Build Monicelli!

add_subdirectory(src)

