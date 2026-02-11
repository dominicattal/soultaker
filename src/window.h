#ifndef WINDOW_H
#define WINDOW_H

#include "util.h"

typedef enum {
    CTRL_INTERACT = 0,
    CTRL_TEST1,
    CTRL_TEST2,
    CTRL_TEST3,
    CTRL_TEST4,
    CTRL_TEST5,
    CTRL_TEST6,
    NUM_CONTROLS
} ControlEnum;

typedef void (*ControlCallbackFuncPtr)(void);

void window_init(void);
void window_update(void);
void window_close(void);
bool window_closed(void);
void window_cleanup(void);

bool window_get_key(i32 key);
bool window_get_mouse_button(i32 button);

i32 window_resolution_x(void);
i32 window_resolution_y(void);
f32 window_aspect_ratio(void);
i32 window_width(void);
i32 window_height(void);
f64 window_cursor_position_x(void);
f64 window_cursor_position_y(void);
vec2 window_cursor_position(void);
void window_toggle_fullscreen(void);

#endif
