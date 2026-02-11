#include "internal.h"
#include "../gui.h"
#include "../game.h"
#include "../event.h"
#include <stdio.h>
#include <stdlib.h>

extern WindowContext window_context;

static i32 control_cmp(Control* ctrl1, Control* ctrl2)
{
    if (ctrl1->glfw_enum < ctrl2->glfw_enum)
        return -1;
    if (ctrl1->glfw_enum > ctrl2->glfw_enum)
        return 1;
    if (ctrl1->mods < ctrl2->mods)
        return -1;
    if (ctrl1->mods > ctrl2->mods)
        return 1;
    return 0;
}

Control* control_get(ControlEnum ctrl_enum)
{
    return &window_context.controls[window_context.control_idx_map[ctrl_enum]];
}

bool control_find_range(i32 glfw_enum, i32 mods, i32* l, i32* r)
{
    Control test = (Control) {
        .glfw_enum = glfw_enum,
        .mods = mods
    };
    Control* ctrl;
    i32 ll, rr, m, cmp;
    ll = 0;
    rr = NUM_CONTROLS-1;
    while (ll <= rr) {
        m = ll + (rr-ll) / 2;
        ctrl = &window_context.controls[m];
        cmp = control_cmp(&test, ctrl);
        if (cmp == 1)
            ll = m + 1;
        else if (cmp == -1)
            rr = m - 1;
        else
            goto found;
    }
    return false;
found:
    ll = rr = m;
    while (ll > 0) {
        ctrl = &window_context.controls[ll-1];
        if (control_cmp(&test, ctrl) != 0)
            break;
        ll--;
    }
    while (rr < NUM_CONTROLS-1) {
        ctrl = &window_context.controls[rr+1];
        if (control_cmp(&test, ctrl) != 0)
            break;
        rr++;
    }
    *l = ll;
    *r = rr;
    return true;
}

void window_set_control_binding(ControlEnum ctrl_enum, i32 glfw_enum, i32 mods)
{
    i32 idx = window_context.control_idx_map[ctrl_enum];
    Control* ctrl = &window_context.controls[idx];
    Control key = window_context.controls[idx];
    key.glfw_enum = glfw_enum;
    key.mods = mods;
    if (control_cmp(&key, ctrl) == 1) {
        while (idx < NUM_CONTROLS-1 && control_cmp(&key, &window_context.controls[idx+1]) == 1) {
            ctrl = &window_context.controls[idx+1];
            window_context.controls[idx] = *ctrl;
            window_context.control_idx_map[ctrl->ctrl_enum] = idx;
            idx++;
        }
    } else {
        while (idx > 0 && control_cmp(&key, &window_context.controls[idx-1]) == -1) {
            ctrl = &window_context.controls[idx-1];
            window_context.controls[idx] = *ctrl;
            window_context.control_idx_map[ctrl->ctrl_enum] = idx;
            idx--;
        }
    }
    window_context.controls[idx] = key;
    window_context.control_idx_map[ctrl_enum] = idx;
}

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

void window_control_callback(i32 glfw_enum, i32 action, i32 mods)
{
    Control* ctrl;
    i32 l, r, res;
    res = control_find_range(glfw_enum, mods, &l, &r);
    if (!res)
        return;
    while (l <= r) {
        ctrl = &window_context.controls[l++];
        game_control_callback(ctrl->ctrl_enum, action);
        event_create_gui_control_callback(ctrl->ctrl_enum, action);
    }
}

void window_mouse_button_callback(GLFWwindow* window, i32 button, i32 action, i32 mods)
{
    event_create_gui_mouse_button_callback(button, action, mods);
    window_control_callback(button, action, mods);
}

void window_cursor_pos_callback(GLFWwindow* window, f64 xpos, f64 ypos)
{
    window_context.cursor.x = xpos;
    window_context.cursor.y = window_context.height - ypos;
    event_create_gui_cursor_pos_callback(window_context.cursor.x, window_context.cursor.y);
}

void window_key_callback(GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods)
{
    if (key == GLFW_KEY_F11 && action == GLFW_PRESS)
        window_toggle_fullscreen();
    event_create_gui_key_callback(key, scancode, action, mods);
    game_key_callback(key, scancode, action, mods);
    window_control_callback(key, action, mods);
}

void window_char_callback(GLFWwindow* window, u32 codepoint)
{
    event_create_gui_char_callback(codepoint);
}

void window_error_callback(i32 x, const char* message) 
{ 
    log_write(FATAL, "GLFW error %x %s", x, message);
}

