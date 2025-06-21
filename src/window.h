#ifndef WINDOW_H
#define WINDOW_H

#include "util.h"

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

#endif
