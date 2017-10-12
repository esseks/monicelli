# Copyright 2017 the Monicelli project authors. All rights reserved.
# Use of this source code is governed by a GPLv3 license, see LICENSE.txt.

cmake_minimum_required(VERSION 3.0)
project(Monicelli)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

find_package(Doxygen)

add_subdirectory(src)

if (DOXYGEN_FOUND)
  set(DOXYGEN_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
    ${DOXYGEN_CONFIG}
    @ONLY
  )

  add_custom_target(doc
    DEPENDS ${DOXYGEN_CONFIG}
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_CONFIG}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    VERBATIM
  )
endif()

install(FILES README.md LICENSE.txt DESTINATION doc/monicelli)
