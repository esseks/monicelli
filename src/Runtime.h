#ifndef RUNTIME_H
#define RUNTIME_H

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

#include <stdint.h>

typedef int8_t Monicelli_Bool;
typedef int8_t Monicelli_Char;
typedef int64_t Monicelli_Int;
typedef float Monicelli_Float;
typedef double Monicelli_Double;

#ifdef __cplusplus
extern "C" {
#endif

void __Monicelli_putBool(Monicelli_Bool value);
void __Monicelli_putChar(Monicelli_Char value);
void __Monicelli_putInt(Monicelli_Int value);
void __Monicelli_putFloat(Monicelli_Float value);
void __Monicelli_putDouble(Monicelli_Double value);

Monicelli_Bool __Monicelli_getBool();
Monicelli_Char __Monicelli_getChar();
Monicelli_Int __Monicelli_getInt();
Monicelli_Float __Monicelli_getFloat();
Monicelli_Double __Monicelli_getDouble();

void __Monicelli_abort();

void __Monicelli_assert(Monicelli_Bool condition);

#ifdef __cplusplus
}
#endif

#endif
