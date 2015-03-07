#ifndef RUNTIME_H
#define RUNTIME_H

#include <stdint.h>

typedef int8_t __Monicelli_Bool;
typedef int8_t __Monicelli_Char;
typedef int64_t __Monicelli_Int;
typedef float __Monicelli_Float;
typedef double __Monicelli_Double;

#ifdef __cplusplus
extern "C" {
#endif

void __Monicelli_putBool(__Monicelli_Bool value);
void __Monicelli_putChar(__Monicelli_Char value);
void __Monicelli_putInt(__Monicelli_Int value);
void __Monicelli_putFloat(__Monicelli_Float value);
void __Monicelli_putDouble(__Monicelli_Double value);

__Monicelli_Bool __Monicelli_getBool();
__Monicelli_Char __Monicelli_getChar();
__Monicelli_Int __Monicelli_getInt();
__Monicelli_Float __Monicelli_getFloat();
__Monicelli_Double __Monicelli_getDouble();

void __Monicelli_abort();

void __Monicelli_assert(__Monicelli_Bool condition);

#ifdef __cplusplus
}
#endif

#endif
