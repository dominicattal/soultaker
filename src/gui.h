#ifndef GUI_H
#define GUI_H

#include "util.h"

typedef struct {
    bool kill_thread;
    pthread_t thread_id;
} GUIContext;

extern GUIContext gui_context;

typedef struct GUIComp GUIComp;
typedef struct GUIComp {
    u64 info1;
    u64 info2;
    void* data;
    union {
        GUIComp** children;
        char* text;
    };
} GUIComp;

void gui_init(void);
void gui_cleanup(void);
void gui_render(void);
void gui_cursor_pos_callback(f64 xpos, f64 ypos);
bool gui_key_callback(i32 key, i32 scancode, i32 action, i32 mods);
bool gui_mouse_button_callback(i32 button, i32 action, i32 mods);

void gui_comp_init(void);
void gui_comp_cleanup(void);

/* #define MAX_NUM_CHILDREN  255

#define ALIGN_LEFT      0
#define ALIGN_CENTER    1
#define ALIGN_RIGHT     2
#define ALIGN_JUSTIFY   3

#define ALIGN_TOP       0
#define ALIGN_BOTTOM    2

#define HOVER_ON        0
#define HOVER_OFF       1

typedef enum CompID {
    COMP_DEFAULT,
    COMP_TEXTBOX,
    COMP_DEBUG,
    NUM_GUI_COMPS
} CompID;

typedef void GUIComp;

// main api
GUIComp* gui_comp_create(i16 x, i16 y, i16 w, i16 h, CompID id);
void gui_comp_init(void);
void gui_comp_attach(GUIComp* parent, GUIComp* child);
void gui_comp_detach(GUIComp* parent, GUIComp* child);
void gui_comp_destroy(GUIComp* comp);
void gui_comp_destroy_children(GUIComp* comp);
void gui_comp_detach_and_destroy(GUIComp* parent, GUIComp* child);
void gui_comp_set_text(GUIComp* comp, const char* text);
void gui_comp_insert_char(GUIComp* comp, const char character, i32 idx);
void gui_comp_delete_char(GUIComp* comp, i32 idx);
void gui_comp_hover(GUIComp* comp, bool status);
void gui_comp_click(GUIComp* comp, i32 button, i32 action);
void gui_comp_key(GUIComp* comp, i32 key, i32 scancode, i32 action, i32 mods);
void gui_comp_update(GUIComp* comp, f64 dt);

// specific functionality
void gui_comp_textbox_set_reference(GUIComp* comp, GUIComp* ref);

// Setters for packed info
void gui_comp_set_id(GUIComp* comp, CompID id);
void gui_comp_set_is_text(GUIComp* comp, bool it);
void gui_comp_set_r(GUIComp* comp, u8 r);
void gui_comp_set_g(GUIComp* comp, u8 g);
void gui_comp_set_b(GUIComp* comp, u8 b);
void gui_comp_set_a(GUIComp* comp, u8 a);
void gui_comp_set_x(GUIComp* comp, i32 x);
void gui_comp_set_y(GUIComp* comp, i32 y);
void gui_comp_set_w(GUIComp* comp, i32 w);
void gui_comp_set_h(GUIComp* comp, i32 h);
void gui_comp_set_num_children(GUIComp* comp, i32 num_children);
void gui_comp_set_halign(GUIComp* comp, u8 ha);
void gui_comp_set_valign(GUIComp* comp, u8 va);
void gui_comp_set_hoverable(GUIComp* comp, bool hv);
void gui_comp_set_hovered(GUIComp* comp, bool hd);
void gui_comp_set_clickable(GUIComp* comp, bool cl);
void gui_comp_set_visible(GUIComp* comp, bool vs);
void gui_comp_set_color(GUIComp* comp, u8 r, u8 g, u8 b, u8 a);
void gui_comp_set_bbox(GUIComp* comp, i32 x, i32 y, i32 w, i32 h);
void gui_comp_set_position(GUIComp* comp, i32 x, i32 y);
void gui_comp_set_size(GUIComp* comp, i32 w, i32 h);
void gui_comp_set_align(GUIComp* comp, u8 ha, u8 va);
void gui_comp_set_tex(GUIComp* comp, i32 tx);
void gui_comp_set_font(GUIComp* comp, Font ft);
void gui_comp_set_font_size(GUIComp* comp, i32 fs);

// Getters for packed info
void gui_comp_get_id(GUIComp* comp, CompID* id);
void gui_comp_get_is_text(GUIComp* comp, bool* it);
void gui_comp_get_r(GUIComp* comp, u8* r);
void gui_comp_get_g(GUIComp* comp, u8* g);
void gui_comp_get_b(GUIComp* comp, u8* b);
void gui_comp_get_a(GUIComp* comp, u8* a);
void gui_comp_get_x(GUIComp* comp, i32* x);
void gui_comp_get_y(GUIComp* comp, i32* y);
void gui_comp_get_w(GUIComp* comp, i32* w);
void gui_comp_get_h(GUIComp* comp, i32* h);
void gui_comp_get_num_children(GUIComp* comp, i32* num_children);
void gui_comp_get_halign(GUIComp* comp, u8* ha);
void gui_comp_get_valign(GUIComp* comp, u8* va);
void gui_comp_get_hoverable(GUIComp* comp, bool* hv);
void gui_comp_get_hovered(GUIComp* comp, bool* hd);
void gui_comp_get_clickable(GUIComp* comp, bool* cl);
void gui_comp_get_visible(GUIComp* comp, bool* vs);
void gui_comp_get_color(GUIComp* comp, u8* r, u8* g, u8* b, u8* a);
void gui_comp_get_bbox(GUIComp* comp, i32* x, i32* y, i32* w, i32* h);
void gui_comp_get_position(GUIComp* comp, i32* x, i32* y);

// Second set of getters
CompID gui_comp_id(GUIComp* comp);
i32  gui_comp_num_children(GUIComp* comp);
bool gui_comp_is_text(GUIComp* comp);
bool gui_comp_is_hoverable(GUIComp* comp);
bool gui_comp_is_hovered(GUIComp* comp);
bool gui_comp_is_clickable(GUIComp* comp);
bool gui_comp_is_visible(GUIComp* comp); */

#endif
