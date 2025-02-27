#include "internal.h"
#include "../window.h"

extern GameContext game_context;

void game_process_input(void)
{
    if (window_get_key(GLFW_KEY_W) == GLFW_PRESS)
        printf("0");
}

void game_key_callback(i32 key, i32 scancode, i32 action, i32 mods)
{
}

void game_mouse_button_callback(i32 button, i32 action, i32 mods)
{
}
