#include "../game.h"
#include <string.h>
#include "../event.h"

extern GameContext game_context;

void game_framebuffer_size_callback(void)
{
    camera_framebuffer_size_callback();
    game_render_framebuffer_size_callback();
}

void game_key_callback(i32 key, i32 scancode, i32 action, i32 mods)
{
    char* test = "This is a notification";
    char* test2 = "This is a notification that wraps and requires more lines blah blah blah";
    if (game_context.halt_input)
        return;
    if (key == GLFW_KEY_O && action == GLFW_PRESS)
        camera_zoom(1);
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
        camera_zoom(-1);
    if (key == GLFW_KEY_RIGHT_BRACKET && (action == GLFW_PRESS || action == GLFW_REPEAT))
        camera_minimap_zoom(1);
    if (key == GLFW_KEY_LEFT_BRACKET && (action == GLFW_PRESS || action == GLFW_REPEAT))
        camera_minimap_zoom(-1);
    if (key == GLFW_KEY_I && action == GLFW_PRESS)
        game_context.paused = 1-game_context.paused;
    if (key == GLFW_KEY_M && action == GLFW_PRESS)
        event_create_game_swap_weapons();
    if (key == GLFW_KEY_G && action == GLFW_PRESS)
        gui_create_notification(test);
    if (key == GLFW_KEY_H && action == GLFW_PRESS)
        gui_create_notification(test2);
}

void game_mouse_button_callback(i32 button, i32 action, i32 mods)
{
}

void game_control_callback(ControlEnum ctrl, i32 action)
{
}
