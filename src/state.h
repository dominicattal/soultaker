#ifndef STATE_H
#define STATE_H

#include "util.h"
#include <windows.h>

void state_init(void);
void state_loop(void);
void state_cleanup(void);
f32  state_get_dt(void);

void* state_load_function(const char* name);

#endif
