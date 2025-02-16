#include "internal.h"

extern GUIContext gui_context;

void gui_cursor_pos_callback_helper(GUIComp* comp, f64 xpos, f64 ypos)
{
    i32 x, y, w, h;
    bool hovered, in_bounds;
    gui_comp_get_bbox(comp, &x, &y, &w, &h);
    gui_comp_get_hovered(comp, &hovered);

    in_bounds = xpos >= x && xpos <= x + w && ypos >= y && ypos <= y + h;
    if (!hovered && in_bounds)
        gui_comp_hover(comp, HOVER_ON);
    else if (hovered && !in_bounds)
        gui_comp_hover(comp, HOVER_OFF);

    for (i32 i = 0; i < gui_comp_num_children(comp); i++)
        gui_cursor_pos_callback_helper(comp->children[i], xpos, ypos);
}

void gui_cursor_pos_callback(f64 xpos, f64 ypos)
{
    gui_cursor_pos_callback_helper(gui_context.root, xpos, ypos);
}

bool gui_key_callback(i32 key, i32 scancode, i32 action, i32 mods)
{
    return false;
}

bool gui_mouse_button_callback(i32 button, i32 action, i32 mods)
{
    return false;
}
