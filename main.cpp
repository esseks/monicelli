#include "Monicelli.tab.h"

#include <iostream>
#include <cstdlib>

using namespace monicelli;

int lineNumber = 1;
Program *program;


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

int main() {
#if YYDEBUG
    yydebug = 1;
#endif

    program = new Program();
    monicelli_parse();

    program->emit(std::cout);

    return 0;
}

