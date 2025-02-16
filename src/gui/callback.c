#include "internal.h"
#include "../window.h"

extern GUIContext gui_context;

static void gui_cursor_pos_callback_helper(GUIComp* comp, f64 xpos, f64 ypos, i32 position_x, i32 position_y, i32 size_x, i32 size_y)
{
    i32 x, y, w, h;
    u8 halign, valign;
    bool hovered, in_bounds;
    gui_comp_get_bbox(comp, &x, &y, &w, &h);
    gui_comp_get_hovered(comp, &hovered);
    gui_comp_get_align(comp, &halign, &valign);
    
    align_comp_position_x(&position_x, halign, size_x, x, w);
    align_comp_position_y(&position_y, valign, size_y, y, h);

    in_bounds = xpos >= position_x && xpos <= position_x + w 
             && ypos >= position_y && ypos <= position_y + h;

    for (i32 i = 0; i < gui_comp_num_children(comp); i++)
        gui_cursor_pos_callback_helper(comp->children[i], xpos, ypos, position_x, position_y, w, h);
    
    if (!hovered && in_bounds)
        gui_comp_hover(comp, HOVER_ON);
    else if (hovered && !in_bounds)
        gui_comp_hover(comp, HOVER_OFF);
}

void gui_cursor_pos_callback(f64 xpos, f64 ypos)
{
    gui_cursor_pos_callback_helper(gui_context.root, xpos, ypos, 0, 0, 0, 0);
}

static void gui_key_callback_helper(GUIComp* comp, i32 key, i32 scancode, i32 action, i32 mods)
{
    for (i32 i = 0; i < gui_comp_num_children(comp); i++)
        gui_key_callback_helper(comp->children[i], key, scancode, action, mods);
    gui_comp_key(comp, key, scancode, action, mods);
}

bool gui_key_callback(i32 key, i32 scancode, i32 action, i32 mods)
{
    gui_key_callback_helper(gui_context.root, key, scancode, action, mods);
    return false;
}

static bool gui_mouse_button_callback_helper(GUIComp* comp, i32 button, i32 action, i32 mods)
{
    for (i32 i = 0; i < gui_comp_num_children(comp); i++)
        if (gui_mouse_button_callback_helper(comp->children[i], button, action, mods))
            return true;

    i32 x, y, w, h, xpos, ypos;
    bool hovered, in_bounds;
    gui_comp_get_bbox(comp, &x, &y, &w, &h);
    gui_comp_get_hovered(comp, &hovered);
    xpos = window_cursor_position_x();
    ypos = window_cursor_position_y();
    in_bounds = xpos >= x && xpos <= x + w && ypos >= y && ypos <= y + h;

    if (in_bounds)
        gui_comp_click(comp, button, action, mods);

    return in_bounds;
}

bool gui_mouse_button_callback(i32 button, i32 action, i32 mods)
{
    return gui_mouse_button_callback_helper(gui_context.root, button, action, mods);
}
