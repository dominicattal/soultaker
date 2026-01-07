#ifndef WINDOW_INTERNAL_H
#define WINDOW_INTERNAL_H

#include "../window.h"

#define GLFW_ENUM_UNBOUND -1

typedef struct {
    ControlEnum ctrl_enum;
    i32 glfw_enum;
    i32 mods;
} Control;

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
    // ControlEnum -> idx in controls
    i32 control_idx_map[NUM_CONTROLS];
    Control controls[NUM_CONTROLS];
} WindowContext;

extern WindowContext window_context;

void window_error_callback(i32 x, const char* message); 
void window_framebuffer_size_callback(GLFWwindow* window, i32 width, i32 height);
void window_cursor_pos_callback(GLFWwindow* window, f64 xpos, f64 ypos);
void window_mouse_button_callback(GLFWwindow* window, i32 button, i32 action, i32 mods);
void window_key_callback(GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods);
void window_control_callback(i32 glfw_enum, i32 action, i32 mods);
void window_char_callback(GLFWwindow* window, u32 codepoint);

void window_set_control_binding(ControlEnum ctrl, i32 glfw_enum, i32 mods);

// returns whether or not glfw_enum is a valid control, will return 
bool control_find_range(i32 glfw_enum, i32 mods, i32* l, i32* r);
Control* control_get(ControlEnum ctrl_enum);

#endif
