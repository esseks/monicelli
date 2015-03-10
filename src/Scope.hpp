#ifndef SCOPE_HPP
#define SCOPE_HPP

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

#include <boost/optional.hpp>
#include <boost/range/adaptor/reversed.hpp>

#include <vector>
#include <unordered_map>


namespace monicelli {

template<class Key, class Value>
class Scope {
public:
    boost::optional<Value> lookup(Key name) {
        for (auto const& table: boost::adaptors::reverse(tables)) {
            auto result = table.find(name);
            if (result != table.end()) {
                return result->second;
            }
        }

        return boost::none;
    }

    void push(Key const& key, Value const& value) {
        tables.back().insert({key, value});
    }

    void enter() {
        tables.emplace_back();
    }

    void leave() {
        if (!tables.empty()) {
            tables.pop_back();
        }
    }

    void drop() {
        tables.clear();
    }

private:
    std::vector<std::unordered_map<Key, Value>> tables;
};

}

#endif
