#ifndef WINDOW_H
#define WINDOW_H

#include "util.h"

typedef struct {
    GLFWwindow* handle;
    i32 width, height;
    i32 xpos, ypos;
    struct {
        i32 x, y;
    } resolution;
    struct {
        GLFWcursor* handle;
        f64 x, y;
        bool hidden;
    } cursor;
    f64 dt;
} WindowContext;

extern WindowContext window_context;

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

#endif
