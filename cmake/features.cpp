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

/**
 * Minimum program containing all required C++11 features.
 * If a compiler cannot compile this, then it won't compile Monicelli.
 */

#include <memory>
#include <unordered_set>
#include <functional>

#include <vector>
#include <string>

enum class Dummy {
    FOO, BAR, BAZ
};

class Banana {
    int yep() const noexcept {
        return 0;
    }
};

int main() {
    std::unique_ptr<int> foo(new int{0});
    std::vector<int> bar = {1, 2, 3};
    for (int baz: bar) {
        baz += 1;
    }
    char *str = nullptr;
    Banana a;
    Banana b = std::move(a);
    long c = std::stol("100");
}
