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
    execute_process(COMMAND brew --prefix ${_brew_pkg_lower} ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE _brew_path)
    if(EXISTS ${_brew_path})
      set(_brew_cmake_module_path ${CMAKE_MODULE_PATH})
      set(_brew_cmake_program_path ${CMAKE_PROGRAM_PATH})
      set(_brew_cmake_include_path ${CMAKE_INCLUDE_PATH})
      set(_brew_cmake_library_path ${CMAKE_LIBRARY_PATH})

      if(NOT CMAKE_MODULE_PATH)
        list(INSERT CMAKE_MODULE_PATH 0 "${_brew_path}")
      endif()

      if(NOT CMAKE_PROGRAM_PATH AND EXISTS "${_brew_path}/bin")
        list(INSERT CMAKE_PROGRAM_PATH 0 "${_brew_path}/bin")
      endif()

      if(NOT CMAKE_INCLUDE_PATH AND EXISTS "${_brew_path}/include")
        list(INSERT CMAKE_INCLUDE_PATH 0 "${_brew_path}/include")
      endif()

      if(NOT CMAKE_LIBRARY_PATH AND EXISTS "${_brew_path}/lib")
        list(INSERT CMAKE_LIBRARY_PATH 0 "${_brew_path}/lib")
      endif()

      find_package(${_package} ${ARGN})

      set(CMAKE_MODULE_PATH ${_brew_cmake_module_path})
      set(CMAKE_PROGRAM_PATH ${_brew_cmake_program_path})
      set(CMAKE_INCLUDE_PATH ${_brew_cmake_include_path})
      set(CMAKE_LIBRARY_PATH ${_brew_cmake_library_path})

      unset(_brew_cmake_module_path)
      unset(_brew_cmake_program_path)
      unset(_brew_cmake_include_path)
      unset(_brew_cmake_library_path)
    else()
      find_package(${_package} ${ARGN})
    endif()
  else()
    find_package(${_package} ${ARGN})
  endif()
endmacro(find_package_prefer_brew)
