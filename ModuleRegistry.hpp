#ifndef MODULE_REGISTRY_HPP
#define MODULE_REGISTRY_HPP

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

#include "Pointers.hpp"

namespace monicelli {

class FunctionPrototype;

class ModuleRegistry {
public:
    ModuleRegistry();
    ModuleRegistry(ModuleRegistry&) = delete;
    virtual ~ModuleRegistry();

    PointerList<FunctionPrototype> const& getRegisteredFunctions() const;
    void registerFunction(FunctionPrototype *proto);

private:
    struct Private;
    Private *d;
};

ModuleRegistry& getModuleRegistry();
void registerStdLib(ModuleRegistry &);

}

#endif
