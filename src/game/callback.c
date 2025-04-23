#include "internal.h"

extern GameContext game_context;

void game_framebuffer_size_callback(void)
{
    camera_framebuffer_size_callback();
}

void game_key_callback(i32 key, i32 scancode, i32 action, i32 mods)
{
    if (key == GLFW_KEY_O && action == GLFW_PRESS)
        camera_zoom(1);
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
        camera_zoom(-1);
}

void game_mouse_button_callback(i32 button, i32 action, i32 mods)
{
}
