#include "Type.h"
#include "Monicelli.tab.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

int lineNumber = 1;

void yyerror(const char *message) {
    fprintf(stderr, "At line %d: %s\n", lineNumber, message);
    exit(1);
}

void emit(const char *text, ...) {
    va_list args;
    va_start(args, text);

    const char *t;
    for (t = text; t != NULL; t = va_arg(args, const char *)) {
        printf("%s", t);
    }

    va_end(args);
}

int main() {
#if YYDEBUG
    yydebug = 1;
#endif
    return yyparse();
}

