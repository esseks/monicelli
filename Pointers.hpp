#ifndef POINTERS_HPP
#define POINTERS_HPP

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

#include <memory>
#include <vector>

#include <boost/optional.hpp>

namespace monicelli {

// We need these instead of an using decl for compatibility
template <class T>
class Pointer: public std::unique_ptr<T> {
public:
    Pointer(T *p = nullptr): std::unique_ptr<T>(p) {}
};


template<class T>
class PointerList: public std::vector<T*> {
public:
    PointerList() {}
    PointerList(PointerList&) = delete;

    virtual ~PointerList() {
        for (T *element: *this) {
            delete element;
        }
    }
};

}

#endif

