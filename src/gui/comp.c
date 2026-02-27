#include "../gui.h"
#include "../window.h"
#include "../game.h"
#include <assert.h>
#include <string.h>

void gui_comp_init(void)
{ 
    gui_context.root = gui_comp_create(0, 0, window_resolution_x(), window_resolution_y());
    for (i32 i = 0; i < NUM_GUI_EVENT_COMPS; i++)
        gui_context.event_comps[i] = NULL;
    gui_context.root->r = 0;
    gui_context.root->g = 0;
    gui_context.root->b = 0;
    gui_context.root->a = 0;
    gui_context.root->valign = ALIGN_BOTTOM;
    gui_comp_set_flag(gui_context.root, GUI_COMP_FLAG_CLICKABLE, true);
    gui_comp_set_flag(gui_context.root, GUI_COMP_FLAG_HOVERABLE, true);
}

void gui_comp_cleanup(void)
{
    gui_comp_destroy(gui_context.root);
}

GUIComp* gui_comp_create(i16 x, i16 y, i16 w, i16 h)
{
    GUIComp* comp = st_calloc(1, sizeof(GUIComp));
    comp->event_id = GUI_COMP_DEFAULT;
    comp->x = x;
    comp->y = y;
    comp->w = w;
    comp->h = h;
    comp->tex = texture_get_id("color");
    comp->font_size = 16;
    comp->font = FONT_MONOSPACE;
    comp->destroy = NULL;
    gui_comp_set_color(comp, 255, 255, 255, 255);
    gui_comp_set_flag(comp, GUI_COMP_FLAG_VISIBLE, true);
    gui_comp_set_flag(comp, GUI_COMP_FLAG_RELATIVE, true);
    gui_comp_set_flag(comp, GUI_COMP_FLAG_AUTO_FREE_DATA, true);
    return comp;
} 

void gui_comp_set_name(GUIComp* comp, const char* text)
{
    comp->name = string_copy(text);
}

static GUIComp* gui_comp_get_by_name_helper(GUIComp* comp, const char* name)
{
    if (comp->name != NULL && strcmp(comp->name, name) == 0)
        return comp;
    GUIComp* res = NULL;
    GUIComp* cur = NULL;
    for (i32 i = 0; i < comp->num_children; i++) {
        cur = gui_comp_get_by_name_helper(comp->children[i], name);
        if (cur != NULL) {
            if (res == NULL) 
                res = cur;
            else {
                log_write(CRITICAL, "duplicate comp name detected: %s", name);
                res = cur;
            }
        }
    }
    return res;
}

GUIComp* gui_comp_get_by_name(const char* name)
{
    return gui_comp_get_by_name_helper(gui_context.root, name);
}

void gui_comp_set_flag(GUIComp* comp, GUICompFlagEnum flag, bool val)
{
    comp->flags = (comp->flags & ~(1<<flag)) | (val<<flag);
}

void gui_comp_toggle_flag(GUIComp* comp, GUICompFlagEnum flag)
{
    i32 val = 1-((comp->flags>>flag)&1);
    comp->flags = (comp->flags & ~(1<<flag)) | (val<<flag);
}

bool gui_comp_get_flag(GUIComp* comp, GUICompFlagEnum flag)
{
    return (comp->flags >> flag) & 1;
}

void gui_comp_attach(GUIComp* parent, GUIComp* child)
{
    if (parent->children == NULL)
        parent->children = st_malloc(sizeof(GUIComp*));
    else
        parent->children = st_realloc(parent->children, (parent->num_children + 1) * sizeof(GUIComp*));
    parent->children[parent->num_children++] = child;
    child->parent = parent;
}

void gui_comp_detach(GUIComp* parent, GUIComp* child)
{
    if (parent != child->parent)
        log_write(WARNING, "specified component does not match child's parent");
    i32 num_children, i;
    num_children = parent->num_children;
    for (i = 0; i < num_children; i++)
        if (parent->children[i] == child)
            break;
    log_assert(i != num_children, "child not found in parent");
    child->parent = NULL;
    for (; i < num_children - 1; i++)
        parent->children[i] = parent->children[i+1];
    num_children--;
    if (num_children == 0) {
        st_free(parent->children);
        parent->children = NULL;
    } else {
        parent->children = st_realloc(parent->children, num_children * sizeof(GUIComp*));
    }
    parent->num_children = num_children;
}

void gui_comp_destroy(GUIComp* comp)
{
    if (comp->event_id != GUI_COMP_DEFAULT)
        gui_context.event_comps[comp->event_id] = NULL;
    if (comp->destroy != NULL)
        comp->destroy(comp);
    for (i32 i = 0; i < comp->num_children; i++)
        gui_comp_destroy(comp->children[i]);
    if (!gui_comp_get_flag(comp, GUI_COMP_FLAG_POINT_TO_TEXT))
        string_free(comp->text);
    if (comp->name != NULL)
        string_free(comp->name);
    if (gui_comp_get_flag(comp, GUI_COMP_FLAG_AUTO_FREE_DATA))
        st_free(comp->data);
    st_free(comp->children);
    st_free(comp);
}

void gui_comp_destroy_children(GUIComp* comp) {
    for (int i = 0; i < comp->num_children; i++)
        gui_comp_destroy(comp->children[i]);
    st_free(comp->children);
    comp->num_children = 0;
    comp->children = NULL;
}

void gui_comp_detach_and_destroy(GUIComp* parent, GUIComp* child)
{
    gui_comp_detach(parent, child);
    gui_comp_destroy(child);
}

void gui_comp_set_text(GUIComp* comp, char* text)
{
    if (!gui_comp_get_flag(comp, GUI_COMP_FLAG_POINT_TO_TEXT))
        string_free(comp->text);
    gui_comp_set_flag(comp, GUI_COMP_FLAG_POINT_TO_TEXT, false);
    if (text == NULL) {
        comp->text = NULL;
        comp->text_length = 0;
        return;
    }
    comp->text = text;
    comp->text_length = strlen(text);
}

void gui_comp_copy_text(GUIComp* comp, const char* text)
{
    if (!gui_comp_get_flag(comp, GUI_COMP_FLAG_POINT_TO_TEXT))
        string_free(comp->text);
    gui_comp_set_flag(comp, GUI_COMP_FLAG_POINT_TO_TEXT, false);
    if (text == NULL) {
        comp->text = NULL;
        comp->text_length = 0;
        return;
    }
    comp->text = string_copy_len(text, &comp->text_length);
}

void gui_comp_point_to_text(GUIComp* comp, char* text)
{
    if (comp->text == text)
        return;
    if (!gui_comp_get_flag(comp, GUI_COMP_FLAG_POINT_TO_TEXT))
        string_free(comp->text);
    gui_comp_set_flag(comp, GUI_COMP_FLAG_POINT_TO_TEXT, true);
    if (text == NULL) {
        comp->text = NULL;
        comp->text_length = 0;
        return;
    }
    comp->text = text;
    comp->text_length = strlen(text);
}

void gui_comp_remove_text(GUIComp* comp)
{
    comp->text_length = 0;
    string_free(comp->text);
    comp->text = NULL;
}

void gui_comp_insert_char(GUIComp* comp, const char c, i32 idx)
{
    log_assert(idx >= -1, "Invalid index for string insertion %d", idx);
    i32 length = comp->text_length;
    char* new_text = st_malloc((length + 2) * sizeof(char));
    if (idx == -1 || idx >= length) {
        strncpy(new_text, comp->text, length);
        new_text[length] = c;
    } else {
        strncpy(new_text, comp->text, idx);
        new_text[idx] = c;
        strncpy(new_text, comp->text + idx + 1, length - idx + 1);
    }
    new_text[length+1] = '\0';
    st_free(comp->text);
    comp->text = new_text;
    comp->text_length = length+1;
}

void gui_comp_delete_char(GUIComp* comp, i32 idx)
{
    log_assert(idx >= -1, "Invalid index for string deletion %d", idx);
    if (comp->text == NULL) return;
    i32 length = comp->text_length;
    if (length == 1) {
        st_free(comp->text);
        comp->text_length = 0;
        comp->text = NULL;
        return;
    }
    char* new_text = st_malloc(length * sizeof(char));
    if (idx == -1 || idx >= length) {
        strncpy(new_text, comp->text, length-1);
    } else {
        strncpy(new_text, comp->text, idx);
        strncpy(new_text, comp->text + idx, length - idx + 1);
    }
    new_text[length-1] = '\0';
    st_free(comp->text);
    comp->text = new_text;
    comp->text_length--;
}

void gui_comp_hover(GUIComp* comp, bool status)
{
    gui_comp_set_flag(comp, GUI_COMP_FLAG_HOVERED, status);
    if (comp->hover == NULL)
        return;
    ((GUIHoverFPtr)(comp->hover))(comp, status);
}

void gui_comp_click(GUIComp* comp, i32 button, i32 action, i32 mods)
{
    if (comp->click == NULL)
        return;
    ((GUIClickFPtr)(comp->click))(comp, button, action, mods);
}

void gui_comp_key(GUIComp* comp, i32 key, i32 scancode, i32 action, i32 mods)
{
    if (comp->key == NULL)
        return;
    ((GUIKeyFPtr)(comp->key))(comp, key, scancode, action, mods);
}

void gui_comp_control(GUIComp* comp, ControlEnum ctrl, i32 action)
{
    if (comp->control == NULL)
        return;
    ((GUIControlFPtr)(comp->control))(comp, ctrl, action);
}

void gui_comp_update(GUIComp* comp, f32 dt)
{
   if (comp->update == NULL)
       return;
   ((GUIUpdateFPtr)(comp->update))(comp, dt);
}

void gui_update_comps_helper(GUIComp* comp, f32 dt)
{
    gui_comp_update(comp, dt);
    for (i32 i = 0; i < comp->num_children; i++)
        gui_update_comps_helper(comp->children[i], dt);
}

void gui_update_comps(f32 dt)
{
    gui_update_comps_helper(gui_context.root, dt);
}

void align_comp_position_x(i32* position_x, u8 halign, i32 size_x, i32 x, i32 w)
{
    if      (halign == ALIGN_LEFT)       *position_x += x;
    else if (halign == ALIGN_CENTER_POS) *position_x += (size_x - w) / 2 + x;
    else if (halign == ALIGN_CENTER_NEG) *position_x += (size_x - w) / 2 - x;
    else if (halign == ALIGN_RIGHT)      *position_x += size_x - w - x;
}

void align_comp_position_y(i32* position_y, u8 valign, i32 size_y, i32 y, i32 h)
{
    if      (valign == ALIGN_TOP)        *position_y += size_y - h - y;
    else if (valign == ALIGN_CENTER_POS) *position_y += (size_y - h) / 2 + y;
    else if (valign == ALIGN_CENTER_NEG) *position_y += (size_y - h) / 2 - y;
    else if (valign == ALIGN_BOTTOM)     *position_y += y;
}

void gui_comp_get_true_position(GUIComp* comp, i32* x, i32* y)
{
    i32 res_x, res_y;
    res_x = res_y = 0;
    while (comp->parent != NULL) {
        if (comp->halign == ALIGN_LEFT)
            res_x = res_x + comp->x;
        else if  (comp->halign == ALIGN_CENTER_POS)
            res_x = res_x + comp->x + (comp->parent->w - comp->w) / 2;
        else if  (comp->halign == ALIGN_CENTER_NEG)
            res_x = res_x - comp->x + (comp->parent->w - comp->w) / 2;
        else
            res_x = res_x - comp->x - comp->w + comp->parent->w;
        if (comp->valign == ALIGN_TOP)
            res_y = res_y - comp->y + comp->parent->h - comp->h;
        else if  (comp->valign == ALIGN_CENTER_POS)
            res_y = res_y + comp->y - comp->h / 2 + comp->parent->h / 2;
        else if  (comp->valign == ALIGN_CENTER_NEG)
            res_y = res_y - comp->y - comp->h / 2 + comp->parent->h / 2;
        else
            res_y = res_y + comp->y;
        comp = comp->parent;
    }
    *x = res_x;
    *y = res_y;
}

bool gui_comp_contains_cursor(GUIComp* comp)
{
    f64 cx = window_cursor_position_x();
    f64 cy = window_cursor_position_y();
    i32 x, y, w, h;
    w = comp->w;
    h = comp->h;
    gui_comp_get_true_position(comp, &x, &y);
    return cx >= x && cx <= x + w && cy >= y && cy <= y + h;
}

void gui_comp_set_bbox(GUIComp* comp, i32 x, i32 y, i32 w, i32 h)
{
    comp->x = x;
    comp->y = y;
    comp->w = w;
    comp->h = h;
}

void gui_comp_set_color(GUIComp* comp, i32 r, i32 g, i32 b, i32 a)
{
    comp->r = r;
    comp->g = g;
    comp->b = b;
    comp->a = a;
}

void gui_comp_set_align(GUIComp* comp, i32 halign, i32 valign)
{
    comp->halign = halign;
    comp->valign = valign;
}

void gui_comp_set_text_align(GUIComp* comp, i32 halign, i32 valign)
{
    comp->text_halign = halign;
    comp->text_valign = valign;
}

void gui_comp_print(GUIComp* comp)
{
    char* str = string_create("\nx=%d;y=%d;w=%d;h=%d", comp->x, comp->y, comp->w, comp->h);
    log_write(DEBUG, str);
    string_free(str);
}
