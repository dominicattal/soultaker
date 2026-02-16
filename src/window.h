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
