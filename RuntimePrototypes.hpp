#ifndef RUNTIME_PROTOTYPES_HPP
#define RUNTIME_PROTOTYPES_HPP

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
#include <map>
#include <string>
#include <vector>

#define PUT(type, funcname) \
    new Function { \
        new Id {#funcname}, Type::VOID, \
        new PointerList<FunArg> { \
            new FunArg {new Id {"value"}, type, false} \
        }, \
        new PointerList<Statement>{} \
    }

#define GET(type, funcname) \
    new Function { \
        new Id {#funcname}, type, \
        new PointerList<FunArg> {}, \
        new PointerList<Statement>{} \
    }

namespace monicelli {

static const std::map<Type, std::string> PUT_NAMES = {{
    {Type::BOOL, "__Monicelli_putBool"},
    {Type::CHAR, "__Monicelli_putChar"},
    {Type::FLOAT, "__Monicelli_putFloat"},
    {Type::DOUBLE, "__Monicelli_putDouble"},
    {Type::INT, "__Monicelli_putInt"}
}};

static const std::map<Type, std::string> GET_NAMES = {
    {Type::BOOL, "__Monicelli_getBool"},
    {Type::CHAR, "__Monicelli_getChar"},
    {Type::FLOAT, "__Monicelli_getFloat"},
    {Type::DOUBLE, "__Monicelli_getDouble"},
    {Type::INT, "__Monicelli_getInt"}
};

static const std::string ABORT_NAME = "__Monicelli_abort";
static const std::string ASSERT_NAME = "__Monicelli_assert";
static const std::string ENTRYPOINT_NAME = "__Monicelli_main";


static const std::map<std::string, std::vector<Function const*>> STANDARD_MODULES = {
    {"iostream", {
        PUT(Type::BOOL, __Monicelli_putBool),
        PUT(Type::CHAR, __Monicelli_putChar),
        PUT(Type::FLOAT, __Monicelli_putFloat),
        PUT(Type::DOUBLE, __Monicelli_putDouble),
        PUT(Type::INT, __Monicelli_putInt),
        GET(Type::BOOL, __Monicelli_getBool),
        GET(Type::CHAR, __Monicelli_getChar),
        GET(Type::FLOAT, __Monicelli_getFloat),
        GET(Type::DOUBLE, __Monicelli_getDouble),
        GET(Type::INT, __Monicelli_getInt)
    }},
    {"cassert", { new Function{
        new Id("__Monicelli_assert"), Type::VOID,
        new PointerList<FunArg> {
            new FunArg {new Id("condition"), Type::BOOL, false}
        },
        new PointerList<Statement>{}
    }}},
    {"cstdlib", { new Function{
        new Id("__Monicelli_abort"), Type::VOID,
        new PointerList<FunArg> {},
        new PointerList<Statement>{}
    }}}
};

}

#undef PUT
#undef GET

#endif
