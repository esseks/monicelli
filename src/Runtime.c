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

#include "Runtime.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


void __Monicelli_putBool(Monicelli_Bool value) {
    puts(value? "vero\n": "falso\n");
}

void __Monicelli_putChar(Monicelli_Char value) {
    printf("%c", value);
}

void __Monicelli_putInt(Monicelli_Int value) {
    printf("%ld\n", value);
}

void __Monicelli_putFloat(Monicelli_Float value) {
    printf("%g\n", value);
}

void __Monicelli_putDouble(Monicelli_Double value) {
    printf("%lg\n", value);
}

Monicelli_Bool __Monicelli_getBool() {
    Monicelli_Bool tmp;
    printf("%s", "? ");
    scanf("%c", &tmp);
    return tmp != 0? 1: 0;
}

Monicelli_Char __Monicelli_getChar() {
    Monicelli_Char tmp;
    printf("%s", "? ");
    scanf("%c", &tmp);
    return tmp;
}

Monicelli_Int __Monicelli_getInt() {
    Monicelli_Int tmp;
    printf("%s", "? ");
    scanf("%ld", &tmp);
    return tmp;
}

Monicelli_Float __Monicelli_getFloat() {
    Monicelli_Float tmp;
    printf("%s", "? ");
    scanf("%f", &tmp);
    return tmp;
}

Monicelli_Double __Monicelli_getDouble() {
    Monicelli_Double tmp;
    printf("%s", "? ");
    scanf("%lf", &tmp);
    return tmp;
}

void __Monicelli_abort() {
    abort();
}

void __Monicelli_assert(Monicelli_Bool condition) {
    assert(condition);
}

