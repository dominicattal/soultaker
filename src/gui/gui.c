#include "gui.h"
#include <stdlib.h>
#include <stdio.h>

GUI gui;
extern Window window;
extern Component *comp_root;

static float gui_vertices[] = {
     -1.0f, 0.95f, 0.0f, 0.0f, 0.6f,
     -0.85f, 0.95f, 0.0f, 0.0f, 0.6f,
     -1.0f, 1.0f, 0.0f, 0.0f, 0.6f,
     -0.85f, 1.0f, 0.0f, 0.0f, 0.6f,
};

void gui_init(void)
{
    char_map_init();
    gui.max_length = 1000;
    gui.buffer = malloc(gui.max_length * sizeof(f32));
    gui.length = 0;
    comp_root = component_create(0.0f, 0.0f, 1.0f, 1.0f, NO_TEX);
    comp_root->a = 0;
    gui.max_length_changed = TRUE;

    Component *title_card = component_create(0.3, 0.6, 0.4, 0.4 * 2.0/3, SOULTAKER_LOGO_TEX);
    title_card->hoverable = TRUE;
    title_card->id = COMP_START_BUTTON;
    component_attach(comp_root, title_card);

    Component *text_box = component_create(0.02, 0.5, 0.5, 0.1, EMPTY_TEX);
    text_box->a = 0.5;
    text_box->id = COMP_TEXTBOX;
    text_box->sub_id = 0;
    text_box->update_children = FALSE;
    component_add_text(text_box, "THE QUICK BROWN FOX JUMPED OVER THE LAZY DOG 0123456789", 14, 0.5, 0.1);
    component_attach(comp_root, text_box);
    Component *btn = component_create(0.05f, 0.05f, 0.1f, 0.1f, BUTTON_TEX);
    btn->interactable = TRUE;
    btn->hoverable = TRUE;
    btn->id = COMP_BUTTON;
    Component *icon = component_create(0.15, 0.15, 0.7, 0.7, SWORD_1_TEX);
    icon->id = COMP_ICON;
    Component *healthbar = component_create(0.3, 0.05, 0.2, 0.02, NO_TEX);
    healthbar->id = COMP_HEALTHBAR;
    Component *green_part = component_create(0.0, 0.0, 0.75, 1.0, EMPTY_TEX);
    green_part->g = 1.0, green_part->r = 0.0, green_part->b = 0.0;
    Component *red_part = component_create(0.75, 0.0, 0.25, 1.0, EMPTY_TEX);
    red_part->r = 1.0, red_part->g = red_part->b = 0.0;
    component_attach(comp_root, btn);
    component_attach(btn, icon);
    component_attach(comp_root, healthbar);
    component_attach(healthbar, green_part);
    component_attach(healthbar, red_part);
}

#define Z gui.buffer[gui.length++]

void gui_update_data_helper(Component *comp, f32 x, f32 y, f32 w, f32 h)
{
    f32 new_x1, new_y1, new_x2, new_y2, win_x1, win_x2, win_y1, win_y2;
    if (comp->relative) {
        new_x1 = comp->x * w + x, new_x2 = (comp->x + comp->w) * w + x;
        new_y1 = comp->y * h + y, new_y2 = (comp->y + comp->h) * h + y;
    } else {
        new_x1 = comp->x, new_x2 = comp->x + comp->w;
        new_y1 = comp->y, new_y2 = comp->y + comp->h;
    }
    win_x1 = 2 * (new_x1 / (comp == comp_root ? 1 : window.aspect_ratio) - 0.5f), win_y1 = 2 * (new_y1 - 0.5f);
    win_x2 = 2 * (new_x2 / (comp == comp_root ? 1 : window.aspect_ratio) - 0.5f), win_y2 = 2 * (new_y2 - 0.5f);
    if (gui.length + 36 >= gui.max_length) {
        gui.max_length += 1000;
        gui.buffer = realloc(gui.buffer, gui.max_length * sizeof(f32));
        gui.max_length_changed = TRUE;
    }
    Z = win_x1, Z = win_y1, Z = 0.0f, Z = 1.0f, Z = comp->tex, Z = comp->r, Z = comp->g, Z = comp->b, Z = comp->a;
    Z = win_x2, Z = win_y1, Z = 1.0f, Z = 1.0f, Z = comp->tex, Z = comp->r, Z = comp->g, Z = comp->b, Z = comp->a;
    Z = win_x1, Z = win_y2, Z = 0.0f, Z = 0.0f, Z = comp->tex, Z = comp->r, Z = comp->g, Z = comp->b, Z = comp->a;
    Z = win_x1, Z = win_y2, Z = 0.0f, Z = 0.0f, Z = comp->tex, Z = comp->r, Z = comp->g, Z = comp->b, Z = comp->a;
    Z = win_x2, Z = win_y1, Z = 1.0f, Z = 1.0f, Z = comp->tex, Z = comp->r, Z = comp->g, Z = comp->b, Z = comp->a;
    Z = win_x2, Z = win_y2, Z = 1.0f, Z = 0.0f, Z = comp->tex, Z = comp->r, Z = comp->g, Z = comp->b, Z = comp->a;
    for (i32 i = 0; i < comp->num_children; i++)
        gui_update_data_helper(comp->children[i], new_x1, new_y1, new_x2 - new_x1, new_y2 - new_y1);
}

void gui_update_data(void)
{
    gui.length = 0;
    gui_update_data_helper(comp_root, comp_root->x, comp_root->y, comp_root->w, comp_root->h);
}

static bool cursor_in_bounds(f32 x1, f32 x2, f32 y1, f32 y2)
{
    return window.cursor.position.x * window.aspect_ratio >= x1 
           && window.cursor.position.x * window.aspect_ratio <= x2 
           && 1 - window.cursor.position.y >= y1 
           && 1 - window.cursor.position.y <= y2;
}

void gui_update_helper(Component *comp, f32 x, f32 y, f32 w, f32 h)
{
    f32 new_x1, new_y1, new_x2, new_y2, win_x1, win_x2, win_y1, win_y2;
    new_x1 = comp->x * w + x, new_x2 = (comp->x + comp->w) * w + x;
    new_y1 = comp->y * h + y, new_y2 = (comp->y + comp->h) * h + y;
    if (comp->update_children)
        for (i32 i = 0; i < comp->num_children; i++)
            gui_update_helper(comp->children[i], new_x1, new_y1, new_x2 - new_x1, new_y2 - new_y1);
    component_update(comp);
    if (!comp->hoverable)
        return;
    component_hover_callback(comp, cursor_in_bounds(new_x1, new_x2, new_y1, new_y2));
}

bool gui_update(void)
{
    gui_update_helper(comp_root, comp_root->x, comp_root->y, comp_root->w, comp_root->h);
    gui_update_data();
    if (gui.max_length_changed) {
        renderer_malloc(GUI_VAO, gui.max_length);
        gui.max_length_changed = FALSE;
    }
    renderer_update(GUI_VAO, 0, gui.length, gui.buffer);
    return FALSE;
}

void gui_mouse_button_callback_helper(Component *comp, f32 x, f32 y, f32 w, f32 h, i32 button, i32 action)
{
    f32 new_x1, new_y1, new_x2, new_y2, win_x1, win_x2, win_y1, win_y2;
    new_x1 = comp->x * w + x, new_x2 = (comp->x + comp->w) * w + x;
    new_y1 = comp->y * h + y, new_y2 = (comp->y + comp->h) * h + y;
    if (comp->update_children)
        for (i32 i = 0; i < comp->num_children; i++)
            gui_mouse_button_callback_helper(comp->children[i], new_x1, new_y1, new_x2 - new_x1, new_y2 - new_y1, button, action);
    if (comp->interactable && cursor_in_bounds(new_x1, new_x2, new_y1, new_y2))
        component_mouse_button_callback(comp, button, action);
}

void gui_mouse_button_callback(i32 button, i32 action)
{
    gui_mouse_button_callback_helper(comp_root, comp_root->x, comp_root->y, comp_root->w, comp_root->h, button, action);
}

void gui_key_callback_helper(Component *comp, f32 x, f32 y, f32 w, f32 h, i32 key, i32 scancode, i32 action, i32 mods)
{
    f32 new_x1, new_y1, new_x2, new_y2, win_x1, win_x2, win_y1, win_y2;
    new_x1 = comp->x * w + x, new_x2 = (comp->x + comp->w) * w + x;
    new_y1 = comp->y * h + y, new_y2 = (comp->y + comp->h) * h + y;
    if (comp->update_children)
        for (i32 i = 0; i < comp->num_children; i++)
            gui_key_callback_helper(comp->children[i], new_x1, new_y1, new_x2 - new_x1, new_y2 - new_y1, key, scancode, action, mods);
    if (comp->interactable && cursor_in_bounds(new_x1, new_x2, new_y1, new_y2))
        component_key_callback(comp, key, scancode, action, mods);
}

void gui_key_callback(i32 key, i32 scancode, i32 action, i32 mods)
{
    gui_key_callback_helper(comp_root, comp_root->x, comp_root->y, comp_root->w, comp_root->h, key, scancode, action, mods);
}

void gui_destroy(void)
{
    component_destroy(comp_root);
    free(gui.buffer);
}