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

## 1. Find Flex and Bison

find_package(BISON REQUIRED)
find_package(FLEX 2.5 REQUIRED)

if (BISON_VERSION VERSION_LESS 2.5)
    message(FATAL_ERROR "At least Bison 2.5 is required.")
elseif(BISON_VERSION VERSION_LESS 3.0)
    message("== Bison 2.5 was found. You have to apply cmake/bison2.patch...")
endif()

## 2. External components

find_package(Boost 1.48 REQUIRED regex system filesystem)
find_package(LLVM REQUIRED CONFIG)

find_library(YAML_LIBRARIES yaml-cpp)
find_path(YAML_INCLUDE_DIRS yaml.h /usr/include/yaml-cpp/)

add_definitions(
    ${Boost_DEFINITIONS}
    ${LLVM_DEFINITIONS}
)

include_directories(
    ${Boost_INCLUDE_DIRS}
    ${LLVM_INCLUDE_DIRS}
    ${YAML_INCLUDE_DIRS}
)

## 3. Build

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

bison_target(Parser Monicelli.ypp ${CMAKE_CURRENT_BINARY_DIR}/Parser.cpp)
flex_target(Scanner Monicelli.lpp ${CMAKE_CURRENT_BINARY_DIR}/Lexer.cpp)
add_flex_bison_dependency(Scanner Parser)

add_executable(mcc
    main.cpp Nodes.cpp
    ModuleRegistry.cpp ModuleLoader.cpp
    ${BISON_Parser_OUTPUTS} ${FLEX_Scanner_OUTPUTS}
    CppEmitter.cpp BitcodeEmitter.cpp
)

target_compile_options(mcc PRIVATE
    ${LLVM_CXXFLAGS} ${Boost_CXXFLAGS}
    -Wall -Wextra -Werror -Wno-unused-parameter -Wno-deprecated-register
    -std=c++0x -DYYDEBUG=0
)

llvm_map_components_to_libnames(LLVM_LIBRARIES
    support core native bitwriter
)

target_link_libraries(mcc
    ${Boost_LIBRARIES}
    ${LLVM_LIBRARIES}
    ${YAML_LIBRARIES}
)

## 5. Build the runtime library too

add_library(mcrt STATIC Runtime.c)

## 6. Install targets

install(TARGETS mcc DESTINATION bin/)
install(TARGETS mcrt DESTINATION lib/)

