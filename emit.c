#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

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

