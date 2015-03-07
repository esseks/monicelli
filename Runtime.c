#include "Runtime.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


void __Monicelli_main();

void __Monicelli_putBool(__Monicelli_Bool value) {
    puts(value? "vero": "falso");
}

void __Monicelli_putChar(__Monicelli_Char value) {
    printf("%c", value);
}

void __Monicelli_putInt(__Monicelli_Int value) {
    printf("%ld", value);
}

void __Monicelli_putFloat(__Monicelli_Float value) {
    printf("%f", value);
}

void __Monicelli_putDouble(__Monicelli_Double value) {
    printf("%lf", value);
}

__Monicelli_Bool __Monicelli_getBool() {
    __Monicelli_Bool tmp;
    printf("%s", "? ");
    scanf("%c", &tmp);
    return tmp != 0? 1: 0;
}

__Monicelli_Char __Monicelli_getChar() {
    __Monicelli_Char tmp;
    printf("%s", "? ");
    scanf("%c", &tmp);
    return tmp;
}

__Monicelli_Int __Monicelli_getInt() {
    __Monicelli_Int tmp;
    printf("%s", "? ");
    scanf("%ld", &tmp);
    return tmp;
}

__Monicelli_Float __Monicelli_getFloat() {
    __Monicelli_Float tmp;
    printf("%s", "? ");
    scanf("%f", &tmp);
    return tmp;
}

__Monicelli_Double __Monicelli_getDouble() {
    __Monicelli_Double tmp;
    printf("%s", "? ");
    scanf("%lf", &tmp);
    return tmp;
}

void __Monicelli_abort() {
    abort();
}

void __Monicelli_assert(__Monicelli_Bool condition) {
    assert(condition);
}

int main() {
    __Monicelli_main();
}

