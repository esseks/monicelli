#include "Scanner.hpp"
#include "Parser.hpp"

#include <iostream>
#include <fstream>

using namespace monicelli;

int main(int argc, char **argv) {
    Program program;
    Scanner scanner(std::cin);
    Parser parser(scanner, program);

#if YYDEBUG
    parser.set_debug_level(1);
#endif

    parser.parse();
    program.emit(std::cout);

    return 0;
}

