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

include(InstallRequiredSystemLibraries)

set(CPACK_GENERATOR "TGZ;DEB;RPM;ZIP")

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "Monicelli: an esoteric programming language, come se fosse antani."
)
set(CPACK_PACKAGE_VENDOR "Stefano Sanfilippo")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Stefano Sanfilippo")

set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")

set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")

set(CPACK_PACKAGE_INSTALL_DIRECTORY
    "monicelli-${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}"
)

include(CPack)

