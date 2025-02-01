#ifndef GAME_H
#define GAME_H

#include "util.h"

typedef struct {
    bool kill_thread;
    pthread_t thread_id;
} GameContext;

extern GameContext game_context;

void game_init(void);
void game_cleanup(void);
void game_process_input(void);
void game_render(void);
void game_key_callback(i32 key, i32 scancode, i32 action, i32 mods);
void game_mouse_button_callback(i32 button, i32 action, i32 mods);

#endif
