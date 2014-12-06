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

default: compile cleanautogen

bison2: patch2 default unpatch2

compile:
	bison Monicelli.ypp
	flex Monicelli.lpp
	$(CXX) \
    -Wall -Wno-deprecated-register -std=c++0x -DYYDEBUG=0 -O2 \
    Parser.cpp lex.yy.cc Nodes.cpp main.cpp ProgramOptions.cpp -o mcc

patch2:
	# Bison 2 compatibility patch
	patch -r - -p 1 -N < bison2.patch || true

unpatch2:
	patch -p 1 -R < bison2.patch

graph:
	bison --graph Monicelli.y

cleanautogen:
	rm -f Parser.?pp lex.* location.hh position.hh stack.hh

clean: cleanautogen
	rm -f Parser.output Parser.dot
