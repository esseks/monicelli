#include "Monicelli.tab.h"

#include <iostream>
#include <fstream>

#include <cstdlib>
#include <cstdio>

using namespace monicelli;

int lineNumber = 1;
Program *program;

extern FILE *monicelli_in;


void monicelli_error(const char *message) {
    std::cerr << "At line " << lineNumber << ": " << message << std::endl;
    std::exit(1);
}

void monicelli_meta(const char *text) {
    while (text != '\0' && *text == ' ') {
        text += 1;
    }
    std::cerr << "META: " << text << std::endl;
}

int main(int argc, char **argv) {
#if YYDEBUG
    yydebug = 1;
#endif

    bool fromFile = argc > 1;
    bool toFile = argc > 2;

    program = new Program();

    if (fromFile) {
        monicelli_in = fopen(argv[1], "r");
    }
    monicelli_parse();
    if (fromFile) {
        fclose(monicelli_in);
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

