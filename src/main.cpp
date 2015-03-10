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
#include "CLineParser.hpp"

#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_os_ostream.h>

#include <boost/regex.hpp>
#include <boost/filesystem.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <functional>

using namespace monicelli;

boost::regex NAME_RE("^(.+)\\.mc$");
boost::regex MODULE_RE("^(.+)\\.mm$");

int process(std::string const&, std::function<bool(std::ostream&, Program*)>);


int main(int argc, char **argv) {
    parseCommandLine(argc, argv);
    registerStdLib(getModuleRegistry());

    if (!configHas("input")) {
        std::cerr << "No input." << std::endl;
        return 0;
    }

    if (configHas("c++")) {
        return process("cpp", [](std::ostream &outstream, Program *program) {
            CppEmitter emitter(&outstream);
            if (!program->emit(&emitter)) return false;
            return true;
        });
    } else {
        return process("bc", [](std::ostream & outstream, Program *program) {
            BitcodeEmitter emitter;
            if (!program->emit(&emitter)) return false;

            llvm::raw_os_ostream stream(outstream);
            llvm::WriteBitcodeToFile(&emitter.getModule(), stream);

            return true;
        });
    }
}

int process(std::string const& suffix, std::function<bool(std::ostream&, Program*)> writer) {
    std::vector<std::string> sources;
    std::vector<std::string> modules;

    for (std::string const& arg: config<std::vector<std::string>>("input")) {
        if (boost::regex_match(arg, NAME_RE)) {
            sources.push_back(arg);
        } else if (boost::regex_match(arg, MODULE_RE)) {
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

#    if YYDEBUG
        parser.set_debug_level(1);
#    endif

        parser.parse();

        std::string outputname = boost::filesystem::path(name).filename().native();

        if (boost::regex_match(outputname, NAME_RE)) {
            outputname = boost::regex_replace(outputname, NAME_RE, "$1." + suffix);
        } else {
            outputname = outputname + '.' + suffix;
        }

        std::ofstream outstream(outputname);

        if (!writer(outstream, &program)) return 1;
    }

    return 0;
}

