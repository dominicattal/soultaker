#include "internal.h"
#include "../window.h"

extern GUIContext gui_context;

void gui_framebuffer_size_callback(i32 width, i32 height)
{
    if (pthread_equal(pthread_self(), gui_context.thread_id)) {
        gui_comp_set_size(gui_context.root, width, height);
    } else {
        GUIEvent event;
        event.type = GUI_EVENT_FRAMEBUFFER_SIZE_CALLBACK;
        event.args.width = width;
        event.args.height = height;
        gui_event_enqueue(&gui_context.event_queue, event);
    }
}

static bool gui_cursor_pos_callback_helper(GUIComp* comp, f64 xpos, f64 ypos, i32 position_x, i32 position_y, i32 size_x, i32 size_y)
{
    i32 x, y, w, h;
    u8 halign, valign;
    bool hovered, hoverable, child_hovered, in_bounds;
    gui_comp_get_bbox(comp, &x, &y, &w, &h);
    gui_comp_get_hovered(comp, &hovered);
    gui_comp_get_hoverable(comp, &hoverable);
    gui_comp_get_align(comp, &halign, &valign);
    
    align_comp_position_x(&position_x, halign, size_x, x, w);
    align_comp_position_y(&position_y, valign, size_y, y, h);

    in_bounds = xpos >= position_x && xpos <= position_x + w 
             && ypos >= position_y && ypos <= position_y + h;

    child_hovered = false;
    for (i32 i = 0; i < gui_comp_num_children(comp); i++)
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
    if (gui_context.thread_id == pthread_self()) {
        bool comp_found = false;
        comp_found = gui_cursor_pos_callback_helper(gui_context.root, xpos, ypos, 0, 0, 0, 0);
        return comp_found;
    } else {
        GUIEvent event;
        event.type = GUI_EVENT_CURSOR_POS_CALLBACK;
        event.args.xpos = xpos;
        event.args.ypos = ypos;
        gui_event_enqueue(&gui_context.event_queue, event);
        return true;
    }
}

static bool gui_key_callback_helper(GUIComp* comp, i32 key, i32 scancode, i32 action, i32 mods)
{
    for (i32 i = 0; i < gui_comp_num_children(comp); i++)
        gui_key_callback_helper(comp->children[i], key, scancode, action, mods);
    gui_comp_key(comp, key, scancode, action, mods);
    return false;
}

static void process_typing_input(i32 key, i32 scancode, i32 action, i32 mods)
{
    if (key == GLFW_KEY_BACKSPACE && action != GLFW_RELEASE) {
        gui_comp_delete_char(gui_context.typing_comp, -1);
    }
}

bool gui_key_callback(i32 key, i32 scancode, i32 action, i32 mods)
{
    if (gui_context.thread_id == pthread_self()) {
        bool comp_found = true;
        if (gui_context.typing_comp == NULL)
            comp_found = gui_key_callback_helper(gui_context.root, key, scancode, action, mods);
        else
            process_typing_input(key, scancode, action, mods);
        return comp_found;
    } else {
        GUIEvent event;
        event.type = GUI_EVENT_KEY_CALLBACK;
        event.args.key = key;
        event.args.scancode = scancode;
        event.args.action = action;
        event.args.mods = mods;
        gui_event_enqueue(&gui_context.event_queue, event);
        return true;
    }
}

static bool gui_mouse_button_callback_helper(GUIComp* comp, i32 xpos, i32 ypos, i32 button, i32 action, i32 mods, i32 position_x, i32 position_y, i32 size_x, i32 size_y)
{
    i32 x, y, w, h;
    u8 halign, valign;
    bool clickable, child_clicked, in_bounds;
    gui_comp_get_bbox(comp, &x, &y, &w, &h);
    gui_comp_get_clickable(comp, &clickable);
    gui_comp_get_align(comp, &halign, &valign);
    
    align_comp_position_x(&position_x, halign, size_x, x, w);
    align_comp_position_y(&position_y, valign, size_y, y, h);

    in_bounds = xpos >= position_x && xpos <= position_x + w 
             && ypos >= position_y && ypos <= position_y + h;

    child_clicked = false;
    for (i32 i = 0; i < gui_comp_num_children(comp); i++)
        if (gui_mouse_button_callback_helper(comp->children[i], xpos, ypos, button, action, mods, position_x, position_y, w, h))
            child_clicked = true;

    if (!clickable)
        return false;
    
    if (!child_clicked && in_bounds)
        gui_comp_click(comp, button, action, mods);

    return in_bounds || child_clicked;
}

bool gui_mouse_button_callback(i32 button, i32 action, i32 mods)
{
    if (gui_context.thread_id == pthread_self()) {
        f64 xpos, ypos;
        bool comp_found = false;
        xpos = window_cursor_position_x();
        ypos = window_cursor_position_y();
        comp_found = gui_mouse_button_callback_helper(gui_context.root, xpos, ypos, button, action, mods, 0, 0, 0, 0);
        return comp_found;
    } else {
        GUIEvent event;
        event.type = GUI_EVENT_MOUSE_BUTTON_CALLBACK;
        event.args.xpos = window_cursor_position_x();
        event.args.ypos = window_cursor_position_y();
        event.args.button = button;
        event.args.action = action;
        event.args.mods = mods;
        gui_event_enqueue(&gui_context.event_queue, event);
        return true;
    }
}

bool gui_char_callback(u32 codepoint)
{
    if (gui_context.thread_id == pthread_self()) {
        bool comp_found = false;
        if (gui_context.typing_comp != NULL)
            gui_comp_insert_char(gui_context.typing_comp, codepoint, -1);
        return comp_found;
    } else {
        GUIEvent event;
        event.type = GUI_EVENT_CHAR_CALLBACK;
        event.args.codepoint = codepoint;
        gui_event_enqueue(&gui_context.event_queue, event);
        return true;
    }
}

