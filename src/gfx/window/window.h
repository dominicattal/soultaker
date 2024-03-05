#ifndef WINDOW_H
#define WINDOW_H

#include <glad.h>
#include <glfw.h>
#include <gtype.h>
#include <gvec.h>

#define DEFAULT_WINDOW_WIDTH 540
#define DEFAULT_WINDOW_HEIGHT 540

typedef struct Mouse {
    vec2u position;
} Mouse;

typedef struct Window {
    GLFWwindow* handle;
    vec2u size;
    Mouse mouse;
    f32 last_frame, dt, fps;
} Window;

extern Window window;

void window_init(void);
void window_calc_dt(void);

/* abstractions */
i2 window_closed(void);
void window_close(void);
void window_poll_events(void);
void window_swap_buffers(void);
i2 window_key_pressed(GLenum key);

#endif