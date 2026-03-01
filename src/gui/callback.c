#include "../gui.h"
#include "../window.h"

extern GUIContext gui_context;

void gui_framebuffer_size_callback(i32 width, i32 height)
{
    gui_context.root->w = width;
    gui_context.root->h = height;
}

static bool gui_cursor_pos_callback_helper(GUIComp* comp, f64 xpos, f64 ypos, i32 position_x, i32 position_y, i32 size_x, i32 size_y)
{
    i32 x, y, w, h;
    u8 halign, valign;
    bool hovered, hoverable, child_hovered, in_bounds;
    
    x = comp->x;
    y = comp->y;
    w = comp->w;
    h = comp->h;
    halign = comp->halign;
    valign = comp->valign;
    hovered = gui_comp_get_flag(comp, GUI_COMP_FLAG_HOVERED);
    hoverable = gui_comp_get_flag(comp, GUI_COMP_FLAG_HOVERABLE);
    
    align_comp_position_x(&position_x, halign, size_x, x, w);
    align_comp_position_y(&position_y, valign, size_y, y, h);

    in_bounds = xpos >= position_x && xpos <= position_x + w 
             && ypos >= position_y && ypos <= position_y + h;

    child_hovered = false;
    for (i32 i = 0; i < comp->num_children; i++)
        if (gui_cursor_pos_callback_helper(comp->children[i], xpos, ypos, position_x, position_y, w, h))
            child_hovered = true;

    if (!hoverable)
        return false;
    
    if (!child_hovered && !hovered && in_bounds)
        gui_comp_hover(comp, HOVER_ON);
    else if (child_hovered || (hovered && !in_bounds))
        gui_comp_hover(comp, HOVER_OFF);

    return in_bounds || child_hovered;
}

bool gui_cursor_pos_callback(f64 xpos, f64 ypos)
{
    bool comp_found = false;
    comp_found = gui_cursor_pos_callback_helper(gui_context.root, xpos, ypos, 0, 0, 0, 0);
    return comp_found;
}

static bool gui_key_callback_helper(GUIComp* comp, i32 key, i32 scancode, i32 action, i32 mods)
{
    for (i32 i = 0; i < comp->num_children; i++)
        gui_key_callback_helper(comp->children[i], key, scancode, action, mods);
    gui_comp_key(comp, key, scancode, action, mods);
    return false;
}

static void process_typing_input(i32 key, i32 scancode, i32 action, i32 mods)
{
    if (key == GLFW_KEY_BACKSPACE && action != GLFW_RELEASE) {
        gui_comp_delete_char(gui_get_event_comp(GUI_COMP_TYPING), -1);
    }
    gui_comp_key(gui_get_event_comp(GUI_COMP_TYPING), key, scancode, action, mods);
}

bool gui_key_callback(i32 key, i32 scancode, i32 action, i32 mods)
{
    bool comp_found = true;
    if (gui_get_event_comp(GUI_COMP_TYPING) == NULL)
        comp_found = gui_key_callback_helper(gui_context.root, key, scancode, action, mods);
    else
        process_typing_input(key, scancode, action, mods);
    if (!comp_found)
        game_key_callback(key, scancode, action, mods);
    return comp_found;
}

static bool gui_mouse_button_callback_helper(GUIComp* comp, i32 xpos, i32 ypos, i32 button, i32 action, i32 mods, i32 position_x, i32 position_y, i32 size_x, i32 size_y)
{
    i32 x, y, w, h;
    u8 halign, valign;
    bool child_clicked, in_bounds, allow_child_click;
    x = comp->x;
    y = comp->y;
    w = comp->w;
    h = comp->h;
    halign = comp->halign;
    valign = comp->valign;

    if (!gui_comp_get_flag(comp, GUI_COMP_FLAG_CLICKABLE))
        return false;
    
    align_comp_position_x(&position_x, halign, size_x, x, w);
    align_comp_position_y(&position_y, valign, size_y, y, h);

    in_bounds = gui_comp_get_flag(comp, GUI_COMP_FLAG_ALLOW_OUT_OF_BOUNDS_CLICK);
    in_bounds = in_bounds || (xpos >= position_x && xpos <= position_x + w && ypos >= position_y && ypos <= position_y + h);

    child_clicked = false;
    for (i32 i = 0; i < comp->num_children; i++)
        if (gui_mouse_button_callback_helper(comp->children[i], xpos, ypos, button, action, mods, position_x, position_y, w, h))
            child_clicked = true;
    
    allow_child_click = gui_comp_get_flag(comp, GUI_COMP_FLAG_ALLOW_CHILD_CLICK);
    if (in_bounds && (allow_child_click || !child_clicked))
        gui_comp_click(comp, button, action, mods);

    if (gui_comp_get_flag(comp, GUI_COMP_FLAG_IGNORE_MOUSE_BUTTON)) {
        log_write(DEBUG, "%d %d %d", position_x, in_bounds, child_clicked);
        return false;
    }

    if (in_bounds || child_clicked) {
        log_write(DEBUG, "%d %d %d", position_x, in_bounds, child_clicked);
        gui_comp_print(comp);
    }

    return in_bounds || child_clicked;
}

bool gui_mouse_button_callback(i32 button, i32 action, i32 mods)
{
    f64 xpos, ypos;
    bool comp_found = false;
    xpos = window_cursor_position_x();
    ypos = window_cursor_position_y();
    comp_found = gui_mouse_button_callback_helper(gui_context.root, xpos, ypos, button, action, mods, 0, 0, 0, 0);
    game_mouse_button_callback(button, action, mods);
    return comp_found;
}

bool gui_char_callback(u32 codepoint)
{
    bool comp_found = false;
    if (gui_get_event_comp(GUI_COMP_TYPING) != NULL)
        gui_comp_insert_char(gui_get_event_comp(GUI_COMP_TYPING), codepoint, -1);
    return comp_found;
}

static void gui_control_callback_helper(GUIComp* comp, ControlEnum ctrl, i32 action)
{
    for (i32 i = 0; i < comp->num_children; i++)
        gui_control_callback_helper(comp->children[i], ctrl, action);
    gui_comp_control(comp, ctrl, action);
}

void gui_control_callback(ControlEnum ctrl, i32 action)
{
    if (gui_get_event_comp(GUI_COMP_TYPING) != NULL)
        return;
    gui_control_callback_helper(gui_context.root, ctrl, action);
}
