/*
 * Monicelli: an esoteric language compiler
 * 
 * Copyright (C) 2014 Stefano Sanfilippo
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Nodes.hpp"
#include "RuntimePrototypes.hpp"
#include <string>

using namespace monicelli;

Function *monicelli::makeMain(PointerList<Statement> *body) {
    PointerList<FunArg> *noargs = new PointerList<FunArg>();

    return new Function(
        new Id(new std::string(ENTRYPOINT_NAME)),
        Type::VOID, noargs, body
    );
}

