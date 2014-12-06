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

#include "Scanner.hpp"
#include "Parser.hpp"
#include "ProgramOptions.hpp"

#include <iostream>
#include <fstream>

using namespace monicelli;

int main(int argc, char **argv) {
    Program program;
    ProgramOptions programOptions(argc, argv);

    std::ifstream input("/dev/stdin");
    std::ofstream output("/dev/stdout");

    // Chain everything and parse.
    programOptions.
        addOption("--input", "-i", "Input file", "input").
        addOption("--output", "-o", "Output file", "output").
        parse();

    if (programOptions.optionParsed("input")) {
        std::string inputString = programOptions.getValueAsString("input");
        if (inputString != "-")
            input = std::ifstream(inputString);
    }

    if (programOptions.optionParsed("output")) {
        std::string outputString = programOptions.getValueAsString("output");
        if (outputString != "-")
            output = std::ofstream(outputString);
    }

    Scanner scanner(dynamic_cast<std::istream &>(input));
    Parser parser(scanner, program);

#if YYDEBUG
    parser.set_debug_level(1);
#endif

    parser.parse();
    program.emit(dynamic_cast<std::ostream &>(output));

    return 0;
}

