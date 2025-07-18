#include "internal.h"
#include "../event.h"

extern GameContext game_context;

void game_framebuffer_size_callback(void)
{
    camera_framebuffer_size_callback();
}

void game_key_callback(i32 key, i32 scancode, i32 action, i32 mods)
{
    if (game_context.halt_input)
        return;
    if (key == GLFW_KEY_O && action == GLFW_PRESS)
        camera_zoom(1);
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
        camera_zoom(-1);
    if (key == GLFW_KEY_H && action == GLFW_PRESS)
        game_context.paused = 1-game_context.paused;
    if (key == GLFW_KEY_M && action == GLFW_PRESS)
        event_create_game_swap_weapons();
}

void game_mouse_button_callback(i32 button, i32 action, i32 mods)
{
}
