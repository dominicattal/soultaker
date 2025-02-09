#ifndef WINDOW_H
#define WINDOW_H

#include "util.h"

void window_init(void);
void window_update(void);
void window_close(void);
bool window_closed(void);
void window_cleanup(void);

void window_error_callback(i32 x, const char* message); 
void window_framebuffer_size_callback(GLFWwindow* window, i32 width, i32 height);
void window_mouse_button_callback(GLFWwindow* window, i32 button, i32 action, i32 mods);
void window_cursor_pos_callback(GLFWwindow* window, f64 xpos, f64 ypos);
void window_key_callback(GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods);

i32 window_resolution_x(void);
i32 window_resolution_y(void);
i32 window_width(void);
i32 window_height(void);

#endif
