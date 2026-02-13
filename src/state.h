#ifndef STATE_H
#define STATE_H

#include "util.h"
#include "config.h"

typedef struct StateContext {
    Config* config;
    pthread_mutex_t mutex;
    void* handle;
    f32 dt;
} StateContext;

extern StateContext state_context;

void state_init(void);
void state_loop(void);
void state_cleanup(void);
f32  state_dt(void);

void* state_load_function(const char* name);

#endif
