#include "Monicelli.tab.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

int lineNumber = 1;

void yyerror(const char *message) {
    fprintf(stderr, "At line %d: %s\n", lineNumber, message);
    exit(1);
}

#define _(...) emit(__VA_ARGS__, NULL);

void emit(const char *text, ...) {
    va_list args;
    va_start(args, text);

    const char *t;
    for (t = text; t != NULL; t = va_arg(args, const char *)) {
        printf("%s", t);
    }

    va_end(args);
}

void meta(const char *text) {
    while (text != '\0' && *text == ' ') {
        text += 1;
    }
    fprintf(stderr, "META: %s\n", text);
}

int main() {
#if YYDEBUG
    yydebug = 1;
#endif
    return yyparse();
}

