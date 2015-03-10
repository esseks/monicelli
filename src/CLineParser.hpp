#ifndef CLINE_PARSER_HPP
#define CLINE_PARSER_HPP

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

#include <boost/program_options.hpp>

namespace monicelli {

boost::program_options::variables_map const& getConfig();

void parseCommandLine(int argc, char **argv);

template<typename T> inline
T config(std::string const& name) {
    return getConfig()[name].as<T>();
}

static inline
bool configHas(std::string const& name) {
    return getConfig().count(name);
}

}

#endif
