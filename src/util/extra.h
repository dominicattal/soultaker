#ifndef EXTRA_H
#define EXTRA_H

#include "type.h"

#define STRING_CREATE_MAX_LENGTH 1000

void sleep(i32 msec);
f64  get_time(void);
f32  state_dt(void);
char* string_copy(const char* string);
char* string_create(const char* format, ...);
void  string_free(char* string);

#endif
