#ifndef EXTRA_H
#define EXTRA_H

#include "type.h"

void sleep(i32 msec);
f64  get_time(void);

// create a copy of string on the heap
char* string_copy(const char* string);

// string_copy, but return the length of the string in len
char* string_copy_len(const char* string, i32* len);

// create a string with the format that is 
// at most n characters long
char* string_create(const char* fmt, ...);

// string_create, but return the length of the string
char* string_create_len(const char* fmt, i32* len, ...);

// free a string created by string_copy or string_create
void  string_free(char* string);

i32 maxi(i32 x, i32 y);
i32 mini(i32 x, i32 y);
f32 maxf(f32 x, f32 y);
f32 minf(f32 x, f32 y);

f32 lerp(f32 low, f32 high, f32 max_t, f32 t);
f32 randf(void);
f32 randf_range(f32 low, f32 high);
f32 guass_dist(f32 mean, f32 std);
f64 gmodf(f64 x, f64 m);

#endif
