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
#include "CppEmitter.hpp"
#include "ModuleRegistry.hpp"
#include "ModuleLoader.hpp"
#include "BitcodeEmitter.hpp"

#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_os_ostream.h>

#include <boost/regex.hpp>
#include <boost/filesystem.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <regex>

using namespace monicelli;


int main(int argc, char **argv) {
    registerStdLib(getModuleRegistry());

    boost::regex namere("^(.+)\\.mc$");
    boost::regex modulere("^(.+)\\.mm$");

    std::vector<std::string> sources;
    std::vector<std::string> modules;

    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        if (boost::regex_match(arg, namere)) {
            sources.push_back(arg);
        } else if (boost::regex_match(arg, modulere)) {
            modules.push_back(arg);
        }
    }

    for (std::string const& name: modules) {
        loadModule(name, getModuleRegistry());
    }

    for (std::string const& name: sources) {
        std::ifstream instream(name);

        Program program;
        Scanner scanner(&instream);
        Parser parser(scanner, program);
        BitcodeEmitter emitter;

#    if YYDEBUG
        parser.set_debug_level(1);
#    endif

        parser.parse();

        std::string outputname = boost::filesystem::path(name).filename().native();

        if (boost::regex_match(outputname, namere)) {
            outputname = boost::regex_replace(outputname, namere, "$1.bc");
        } else {
            outputname = outputname + ".bc";
        }

        if (!program.emit(&emitter)) return 1;

        std::ofstream outstream(outputname);
        llvm::raw_os_ostream stream(outstream);
        llvm::WriteBitcodeToFile(&emitter.getModule(), stream);
    }
}

