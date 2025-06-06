#ifndef STATE_H
#define STATE_H

#include "util.h"
#include <windows.h>

typedef struct StateContext {
    HMODULE lib;
    f32 dt;
} GlobalContext;

extern GlobalContext global_context;

void state_init(void);
void state_loop(void);
void state_cleanup(void);

#endif
