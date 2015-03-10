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

#include "CLineParser.hpp"

#include <boost/program_options.hpp>
#include <cstdlib>
#include <string>
#include <vector>

namespace po = boost::program_options;
using namespace monicelli;

static po::variables_map CONFIG;
static const std::string VERSION_STRING = 
    "mcc, Monicelli compiler <https://github.com/esseks/monicelli>\n"
    "\n"
    "Copyright © 2014,2015 Stefano Sanfilippo\n"
    "\n"
    "This program comes with ABSOLUTELY NO WARRANTY;\n"
    "This is free software, and you are welcome to redistribute it\n"
    "under certain conditions; See LICENSE.txt for all details"
;

po::variables_map const& monicelli::getConfig() {
    return CONFIG;
}

void monicelli::parseCommandLine(int argc, char **argv) {
    po::options_description desc(
        std::string("Usage: ") + argv[0] + " [options] file.mc ..."
    );
    desc.add_options()
        ("help,h", "display this help message")
        ("version,v", "display version")
        ("input,i", po::value<std::vector<std::string>>(), "input files to process")
    ;

    po::positional_options_description positional;
    positional.add("input", -1);

    po::store(
        po::command_line_parser(argc, argv)
            .options(desc)
            .positional(positional)
            .run(),
        CONFIG
    );

    po::notify(CONFIG);

    if (configHas("help")) {
        std::cout << desc;
        exit(0);
    }

    if (configHas("version")) {
        std::cout << VERSION_STRING << std::endl;
        exit(0);
    }
}
