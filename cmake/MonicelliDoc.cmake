# Copyright 2019 the Monicelli project authors. All rights reserved.
# Use of this source code is governed by a GPLv3 license, see LICENSE.txt.

find_package(Doxygen)

if (DOXYGEN_FOUND)
  set(MONICELLI_DOXYGEN_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
    ${MONICELLI_DOXYGEN_CONFIG}
    @ONLY
  )

  add_custom_target(doc
    DEPENDS ${MONICELLI_DOXYGEN_CONFIG}
    COMMAND ${DOXYGEN_EXECUTABLE} ${MONICELLI_DOXYGEN_CONFIG}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    VERBATIM
  )
endif()

