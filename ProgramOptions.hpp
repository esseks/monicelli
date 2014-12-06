#ifndef PROGRAMOPTIONS_H
#define PROGRAMOPTIONS_H

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

#include <exception>
#include <stdexcept>
#include <vector>
#include <string>
#include <stdint.h>

namespace monicelli {

class Option {

public:
    Option(std::string const & long_value,
           std::string const & short_value,
           std::string const & description,
           std::string const & key) :
        long_value(long_value),
        short_value(short_value),
        description(description),
        key(key) {};

public:
    void setValue(std::string const & value) { this->value = value; }
    std::string const & getRawValue() const { return value;  }
    std::string const & getKey() const { return key ; }

    // We can safely assume an option has been parsed if its value is
    // meaningful (i.e., not empty)
    bool                isValid() const { return not value.empty(); }

public:
    std::string long_value;
    std::string short_value;
    std::string description;

private:
    std::string key;
    std::string value;
};

class OptionNotFoundException : public std::runtime_error {
public:
    OptionNotFoundException(std::string const & error) : std::runtime_error(error) {};
};

class ProgramOptions {

public:
    typedef std::vector<Option>        options;
    typedef options::value_type        value_type;
    typedef options::const_iterator    const_iterator;
    typedef options::reverse_iterator  reverse_iterator;

public:
    ProgramOptions(int argc, char **argv) : argc(argc), argv(argv) {};
    const_iterator begin() const { return valid_options.begin(); }
    const_iterator end() const { return valid_options.end(); }

public:
    // Adds an option to the map of valid ones.
    ProgramOptions & addOption(std::string const & long_value,
                               std::string const & short_value,
                               std::string const & description,
                               std::string const & key);

    uint32_t            getValueAsInt(std::string const & key) const throw (OptionNotFoundException);
    std::string const & getValueAsString(std::string const & key) const throw (OptionNotFoundException);

    // Returns true if an option has been parsed successfully
    bool        optionParsed(std::string const & key);

    bool parse(bool stop_on_error = true);

private:
    int         argc;
    char **     argv;
    options     valid_options;
};

} // namespace

#endif

