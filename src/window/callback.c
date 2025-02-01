#include "../window.h"
#include "../gui.h"
#include "../game.h"
#include <stdio.h>

void window_framebuffer_size_callback(GLFWwindow* window, i32 width, i32 height)
{
}

void window_mouse_button_callback(GLFWwindow* window, i32 button, i32 action, i32 mods)
{
    if (!gui_mouse_button_callback(button, action, mods))
        game_mouse_button_callback(button, action, mods);
}

void window_cursor_pos_callback(GLFWwindow* window, f64 xpos, f64 ypos)
{
    gui_cursor_pos_callback(xpos, ypos);
}

void window_key_callback(GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        window_close();
    if (!gui_key_callback(key, scancode, action, mods))
        game_key_callback(key, scancode, action, mods);
}

void window_error_callback(i32 x, const char* message) 
{ 
    fprintf(stderr, "%d\n%s\n", x, message); 
}
