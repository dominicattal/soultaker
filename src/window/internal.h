#ifndef WINDOW_INTERNAL_H
#define WINDOW_INTERNAL_H

#include "../window.h"

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
    f32 dt;
    GLuint ubo;
} WindowContext;

extern WindowContext window_context;

void window_error_callback(i32 x, const char* message); 
void window_framebuffer_size_callback(GLFWwindow* window, i32 width, i32 height);
void window_mouse_button_callback(GLFWwindow* window, i32 button, i32 action, i32 mods);
void window_cursor_pos_callback(GLFWwindow* window, f64 xpos, f64 ypos);
void window_key_callback(GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods);
void window_char_callback(GLFWwindow* window, u32 codepoint);

#endif
