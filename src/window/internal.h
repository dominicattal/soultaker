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
    f64 dt;
} WindowContext;

extern WindowContext window_context;

#endif
