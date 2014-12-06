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

#include "ProgramOptions.hpp"
#include <iostream>
#include <map>

using namespace monicelli;

ProgramOptions &
ProgramOptions::addOption(std::string const & long_value,
                          std::string const & short_value,
                          std::string const & description,
                          std::string const & key)
{
    valid_options.push_back(Option(long_value, short_value, description, key));
    return *this;
}

uint32_t
ProgramOptions::getValueAsInt(std::string const & key) const throw (OptionNotFoundException)
{
    for (Option const & opt : (*this)) {
        if (opt.getKey() == key) {
            return std::stoi(opt.getRawValue());
        }
    }

    throw OptionNotFoundException("Option does not exist.");
}

std::string const &
ProgramOptions::getValueAsString(std::string const & key) const throw (OptionNotFoundException)
{
    for (Option const & opt : (*this)) {
        if (opt.getKey() == key) {
            return opt.getRawValue();
        }
    }

    throw OptionNotFoundException("Option does not exist.");
}

bool
ProgramOptions::parse(bool stop_on_error)
{
    // Generate an index of the options to speed up the parsing
    std::map<std::string, uint8_t> long_index;
    std::map<std::string, uint8_t> short_index;
    uint8_t counter(0);

    for (Option const & opt : (*this)) {
        long_index [opt.long_value]  = counter;
        short_index[opt.short_value] = counter;

        counter++;
    }

    // Basic parsing of arguments to keep things simple and portable.
    for (int ptr = 1; ptr < argc; ptr++) {
        std::string parsing(argv[ptr]);
        bool found_long = (long_index.find(parsing) != long_index.end());
        bool found_short = (short_index.find(parsing) != short_index.end());

        if (found_long or found_short) {
            uint8_t & index = found_long? long_index[parsing] : short_index[parsing];

            if (++ptr < argc) {
                Option & opt = valid_options[index];
                opt.setValue(argv[ptr]);
            }

            else if (stop_on_error) {
                return false;
            }
        }
    }

    return true;
}

bool
ProgramOptions::optionParsed(std::string const & key)
{
    for (Option const & opt : (*this)) {
        if (opt.getKey() == key) {
            return opt.isValid();
        }
    }

    // No need to throw an exception here.
    return false;
}
