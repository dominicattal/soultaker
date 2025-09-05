#ifndef EXTRA_H
#define EXTRA_H

#include "type.h"

void sleep(i32 msec);
f64  get_time(void);

// create a copy of string on the heap
char* string_copy(const char* string);

// create a string with the format that is 
// at most n characters long
char* string_create(const char* format, int n, ...);

// free a string created by string_copy or string_create
void  string_free(char* string);

f32 maxf(f32 x, f32 y);
f32 minf(f32 x, f32 y);

#endif
