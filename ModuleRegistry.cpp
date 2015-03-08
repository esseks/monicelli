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

#include "ModuleRegistry.hpp"
#include "Nodes.hpp"

#include <vector>

using namespace monicelli;

static ModuleRegistry globalRegistry;

ModuleRegistry& monicelli::getModuleRegistry() {
    return globalRegistry;
}

struct ModuleRegistry::Private {
    PointerList<FunctionPrototype> prototypes;
};

ModuleRegistry::ModuleRegistry() {
    d = new Private;
}

ModuleRegistry::~ModuleRegistry() {
    delete d;
}

PointerList<FunctionPrototype> const& ModuleRegistry::getRegisteredFunctions() const {
    return d->prototypes;
}

void ModuleRegistry::registerFunction(FunctionPrototype *proto) {
    d->prototypes.push_back(proto);
}

#define PUT(type, funcname) \
    new FunctionPrototype { \
        new Id {#funcname}, Type::VOID, \
        new PointerList<FunArg> { \
            new FunArg {new Id {"value"}, type, false} \
        }, \
    }

#define GET(type, funcname) \
    new FunctionPrototype { \
        new Id {#funcname}, type, \
        new PointerList<FunArg> {}, \
    }

void monicelli::registerStdLib(ModuleRegistry &r) {
    r.registerFunction(PUT(Type::CHAR, __Monicelli_putBool));
    r.registerFunction(PUT(Type::CHAR, __Monicelli_putChar));
    r.registerFunction(PUT(Type::FLOAT, __Monicelli_putFloat));
    r.registerFunction(PUT(Type::DOUBLE, __Monicelli_putDouble));
    r.registerFunction(PUT(Type::INT, __Monicelli_putInt));
    r.registerFunction(GET(Type::CHAR, __Monicelli_getBool));
    r.registerFunction(GET(Type::CHAR, __Monicelli_getChar));
    r.registerFunction(GET(Type::FLOAT, __Monicelli_getFloat));
    r.registerFunction(GET(Type::DOUBLE, __Monicelli_getDouble));
    r.registerFunction(GET(Type::INT, __Monicelli_getInt));
    r.registerFunction(new FunctionPrototype {
        new Id("__Monicelli_assert"), Type::VOID,
        new PointerList<FunArg> {
            new FunArg {new Id("condition"), Type::CHAR, false}
        }
    });
    r.registerFunction(new FunctionPrototype {
        new Id("__Monicelli_abort"), Type::VOID,
        new PointerList<FunArg> {}
    });
}

#undef PUT
#undef GET
