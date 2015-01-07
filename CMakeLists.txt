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

# The following macro has been adapted from
# https://gist.github.com/steakknife/c36c99b51703fc6f6c1b

# If is running under Mac OS X and the Homebrew package manager is installed,
# then Homebrew-specific directories for the given package are automatically
# added to the appropriate CMake search paths.

macro(find_package_prefer_brew _package)
  unset(_brew_path)
  unset(_brew_pkg_lower)
  unset(_has_brew)
  find_program(_has_brew NAMES brew DOC "path to Homebrew executable")
  if(_has_brew)
    string(TOLOWER ${_package} _brew_pkg_lower)
    execute_process(COMMAND brew --prefix ${_brew_pkg_lower} ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE _brew_path)
    if(EXISTS ${_brew_path})
      set(BREW_CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})
      set(BREW_CMAKE_PROGRAM_PATH ${CMAKE_PROGRAM_PATH})
      set(BREW_CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH})
      set(BREW_CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH})

      if(NOT CMAKE_MODULE_PATH)
        list(INSERT CMAKE_MODULE_PATH 0 "${_brew_path}")
      endif(NOT CMAKE_MODULE_PATH)

      if(NOT CMAKE_PROGRAM_PATH)
        if(EXISTS "${_brew_path}/bin")
          list(INSERT CMAKE_PROGRAM_PATH 0 "${_brew_path}/bin")
        endif(EXISTS "${_brew_path}/bin")
      endif(NOT CMAKE_PROGRAM_PATH)

      if(NOT CMAKE_INCLUDE_PATH)
        if(EXISTS "${_brew_path}/include")
          list(INSERT CMAKE_INCLUDE_PATH 0 "${_brew_path}/include")
        endif(EXISTS "${_brew_path}/include")
      endif(NOT CMAKE_INCLUDE_PATH)

      if(NOT CMAKE_LIBRARY_PATH)
        if(EXISTS "${_brew_path}/lib")
          list(INSERT CMAKE_LIBRARY_PATH 0 "${_brew_path}/lib")
        endif(EXISTS "${_brew_path}/lib")
      endif(NOT CMAKE_LIBRARY_PATH)

      find_package(${_package} ${ARGN})

      set(CMAKE_MODULE_PATH ${BREW_CMAKE_MODULE_PATH})
      set(CMAKE_PROGRAM_PATH ${BREW_CMAKE_PROGRAM_PATH})
      set(CMAKE_INCLUDE_PATH ${BREW_CMAKE_INCLUDE_PATH})
      set(CMAKE_LIBRARY_PATH ${BREW_CMAKE_LIBRARY_PATH})
    else(EXISTS ${_brew_path})
      find_package(${_package} ${ARGN})
    endif(EXISTS ${_brew_path})
  else(_has_brew)
    find_package(${_package} ${ARGN})
  endif(_has_brew)
endmacro(find_package_prefer_brew)

project(Monicelli)
cmake_minimum_required(VERSION 2.8)

message("== Only a limited set of platforms was tested. We need your help!")
message("== Report build errors to https://github.com/esseks/monicelli/issues")

## 1. Compiler sanity check

try_compile(
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

## 2. Find Flex and Bison

if (CMAKE_HOST_APPLE)
  find_package_prefer_brew(BISON REQUIRED)
  find_package_prefer_brew(FLEX 2.5 REQUIRED)
else (CMAKE_HOST_APPLE)
  find_package(BISON REQUIRED)
  find_package(FLEX 2.5 REQUIRED)
endif (CMAKE_HOST_APPLE)

if (BISON_VERSION VERSION_LESS 2.5)
    message(FATAL_ERROR "At least Bison 2.5 is required.")
elseif(BISON_VERSION VERSION_LESS 3.0)
    message("== Bison 2.5 was found. You have to apply cmake/bison2.patch...")
endif()

## 3. Build

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_definitions(-Wall -Wno-deprecated-register -std=c++0x -DYYDEBUG=0 -O2)

bison_target(Parser Monicelli.ypp ${CMAKE_CURRENT_BINARY_DIR}/Parser.cpp)
flex_target(Scanner Monicelli.lpp ${CMAKE_CURRENT_BINARY_DIR}/Lexer.cpp)
add_flex_bison_dependency(Scanner Parser)

add_executable(
    mcc
    main.cpp Nodes.cpp ${BISON_Parser_OUTPUTS} ${FLEX_Scanner_OUTPUTS}
)

