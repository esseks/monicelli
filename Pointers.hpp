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
#include <initializer_list>

#include <boost/ptr_container/ptr_unordered_set.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

namespace monicelli {

template<typename T> using Pointer = std::unique_ptr<T>;
template<typename T> using PointerList = boost::ptr_vector<T>;
template<typename T> using PointerSet = boost::ptr_unordered_set<T>;

template<typename T>
PointerList<T>* plist_of(std::initializer_list<T*> elements) {
    PointerList<T> *result = new PointerList<T>(elements.size());
    for (T *el: elements) {
        result->push_back(el);
    }
    return result;
}

}

#endif

