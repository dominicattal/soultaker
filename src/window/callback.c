#include "internal.h"
#include "../gui.h"
#include "../game.h"
#include "../event.h"
#include <stdio.h>

extern WindowContext window_context;

void window_framebuffer_size_callback(GLFWwindow* window, i32 width, i32 height)
{
    window_context.width = width;
    window_context.height = height;
    GLfloat aspect_ratio = (GLfloat)window_context.width / window_context.height;
    glViewport(0, 0, window_context.width, window_context.height);
    glBindBuffer(GL_UNIFORM_BUFFER, window_context.ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GLint), &window_context.width);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(GLint), sizeof(GLint), &window_context.height);
    glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(GLint), sizeof(GLfloat), &aspect_ratio);
    event_create_gui_framebuffer_size_callback(width, height);
    game_framebuffer_size_callback();
}

void window_mouse_button_callback(GLFWwindow* window, i32 button, i32 action, i32 mods)
{
    event_create_gui_mouse_button_callback(button, action, mods);
}

void window_cursor_pos_callback(GLFWwindow* window, f64 xpos, f64 ypos)
{
    window_context.cursor.x = xpos;
    window_context.cursor.y = window_context.height - ypos;
    event_create_gui_cursor_pos_callback(window_context.cursor.x, window_context.cursor.y);
}

void window_key_callback(GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        window_close();
    event_create_gui_key_callback(key, scancode, action, mods);
    game_key_callback(key, scancode, action, mods);
}

void window_char_callback(GLFWwindow* window, u32 codepoint)
{
    event_create_gui_char_callback(codepoint);
}

void window_error_callback(i32 x, const char* message) 
{ 
    log_write(FATAL, "GLFW error %x %s", x, message);
}
