#include "Monicelli.tab.hpp"

#include <iostream>
#include <fstream>

#include <cstdlib>
#include <cstdio>

using namespace monicelli;

int lineNumber = 1;
Program *program;

extern FILE *mcin;


void mcerror(const char *message) {
    std::cerr << "At line " << lineNumber << ": " << message << std::endl;
    std::exit(1);
}

void mcmeta(const char *text) {
    while (*text != '\0' && *text == ' ') {
        text += 1;
    }
    std::cerr << "META: " << text << std::endl;
}

int main(int argc, char **argv) {
#if YYDEBUG
    mcdebug = 1;
#endif

    bool fromFile = argc > 1;
    bool toFile = argc > 2;

    program = new Program();

    if (fromFile) {
        mcin = fopen(argv[1], "r");
    }
    mcparse();
    if (fromFile) {
        fclose(mcin);
    }
    if (toFile) {
        std::ofstream out(argv[2]);
        program->emit(out);
        out.close();
    }
    else {
        program->emit(std::cout);
    }
    
    return 0;
}

