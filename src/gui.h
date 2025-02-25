#ifndef GUI_H
#define GUI_H

#include "util.h"
#include "renderer.h"

#define MAX_NUM_CHILDREN  255

#define ALIGN_LEFT          0
#define ALIGN_CENTER        1
#define ALIGN_RIGHT         2
#define ALIGN_JUSTIFY       3
#define ALIGN_TOP           0
#define ALIGN_BOTTOM        2
#define ALIGN_CENTER_POS    1
#define ALIGN_CENTER_NEG    3

#define HOVER_OFF       0
#define HOVER_ON        1

typedef struct GUIData GUIData;
typedef struct GUIComp GUIComp;

typedef enum {
    GUI_PRESET_TEST,
    GUI_PRESET_MAIN_MENU,
    NUM_GUI_PRESETS
} GUIPreset;

typedef void (*GUIHoverFPtr)(GUIComp* comp, bool status); 
typedef void (*GUIClickFPtr)(GUIComp* comp, i32 button, i32 action, i32 mods);
typedef void (*GUIKeyFPtr)(GUIComp* comp, i32 key, i32 scancode, i32 action, i32 mods);

void gui_preset_load(GUIPreset preset);

void gui_init(void);
void gui_cleanup(void);

void gui_render_init(void);
void gui_render(void);
void gui_render_cleanup(void);

bool gui_cursor_pos_callback(f64 xpos, f64 ypos);
bool gui_key_callback(i32 key, i32 scancode, i32 action, i32 mods);
bool gui_mouse_button_callback(i32 button, i32 action, i32 mods);
void gui_comp_hover(GUIComp* comp, bool status);
void gui_comp_click(GUIComp* comp, i32 button, i32 action, i32 mods);
void gui_comp_key(GUIComp* comp, i32 key, i32 scancode, i32 action, i32 mods);

void gui_comp_init(void);
void gui_comp_cleanup(void);
GUIComp* gui_comp_create(i16 x, i16 y, i16 w, i16 h);
void gui_comp_attach(GUIComp* parent, GUIComp* child);
void gui_comp_detach(GUIComp* parent, GUIComp* child);
void gui_comp_destroy(GUIComp* comp);
void gui_comp_destroy_children(GUIComp* comp);
void gui_comp_detach_and_destroy(GUIComp* parent, GUIComp* child);
void gui_comp_set_text(GUIComp* comp, const char* text);
void gui_comp_insert_char(GUIComp* comp, const char character, i32 idx);
void gui_comp_delete_char(GUIComp* comp, i32 idx);
void gui_comp_update(GUIComp* comp, f32 dt);
void gui_comp_add_data(GUIComp* comp, void* data);
void* gui_comp_remove_data(GUIComp* comp);

void gui_comp_set_bbox(GUIComp* comp, i32 x, i32 y, i32 w, i32 h);
void gui_comp_set_position(GUIComp* comp, i32 x, i32 y);
void gui_comp_set_x(GUIComp* comp, i32 x);
void gui_comp_set_y(GUIComp* comp, i32 y);
void gui_comp_set_size(GUIComp* comp, i32 w, i32 h);
void gui_comp_set_w(GUIComp* comp, i32 w);
void gui_comp_set_h(GUIComp* comp, i32 h);
void gui_comp_set_tex(GUIComp* comp, i32 tx);
void gui_comp_set_color(GUIComp* comp, u8 r, u8 g, u8 b, u8 a);
void gui_comp_set_r(GUIComp* comp, u8 r);
void gui_comp_set_g(GUIComp* comp, u8 g);
void gui_comp_set_b(GUIComp* comp, u8 b);
void gui_comp_set_a(GUIComp* comp, u8 a);
void gui_comp_set_is_text(GUIComp* comp, bool it);
void gui_comp_set_hoverable(GUIComp* comp, bool hv);
void gui_comp_set_hovered(GUIComp* comp, bool hd);
void gui_comp_set_clickable(GUIComp* comp, bool cl);
void gui_comp_set_visible(GUIComp* comp, bool vs);
void gui_comp_set_relative(GUIComp* comp, bool rl);
void gui_comp_set_align(GUIComp* comp, u8 ha, u8 va);
void gui_comp_set_halign(GUIComp* comp, u8 ha);
void gui_comp_set_valign(GUIComp* comp, u8 va);
void gui_comp_set_num_children(GUIComp* comp, i32 num_children);
void gui_comp_set_text_align(GUIComp* comp, u8 tha, u8 tva);
void gui_comp_set_text_halign(GUIComp* comp, u8 tha);
void gui_comp_set_text_valign(GUIComp* comp, u8 tva);
void gui_comp_set_font(GUIComp* comp, FontEnum ft);
void gui_comp_set_font_size(GUIComp* comp, i32 fs);

void gui_comp_get_bbox(GUIComp* comp, i32* x, i32* y, i32* w, i32* h);
void gui_comp_get_position(GUIComp* comp, i32* x, i32* y);
void gui_comp_get_x(GUIComp* comp, i32* x);
void gui_comp_get_y(GUIComp* comp, i32* y);
void gui_comp_get_size(GUIComp* comp, i32* w, i32* h);
void gui_comp_get_w(GUIComp* comp, i32* w);
void gui_comp_get_h(GUIComp* comp, i32* h);
void gui_comp_get_tex(GUIComp* comp, i32* tx);
void gui_comp_get_color(GUIComp* comp, u8* r, u8* g, u8* b, u8* a);
void gui_comp_get_r(GUIComp* comp, u8* r);
void gui_comp_get_g(GUIComp* comp, u8* g);
void gui_comp_get_b(GUIComp* comp, u8* b);
void gui_comp_get_a(GUIComp* comp, u8* a);
void gui_comp_get_is_text(GUIComp* comp, bool* it);
void gui_comp_get_hoverable(GUIComp* comp, bool* hv);
void gui_comp_get_hovered(GUIComp* comp, bool* hd);
void gui_comp_get_clickable(GUIComp* comp, bool* cl);
void gui_comp_get_visible(GUIComp* comp, bool* vs);
void gui_comp_get_relative(GUIComp* comp, bool* rl);
void gui_comp_get_align(GUIComp* comp, u8* ha, u8* va);
void gui_comp_get_halign(GUIComp* comp, u8* ha);
void gui_comp_get_valign(GUIComp* comp, u8* va);
void gui_comp_get_num_children(GUIComp* comp, i32* num_children);
void gui_comp_get_text_align(GUIComp* comp, u8* tha, u8* tva);
void gui_comp_get_text_halign(GUIComp* comp, u8* tha);
void gui_comp_get_text_valign(GUIComp* comp, u8* tva);
void gui_comp_get_font(GUIComp* comp, FontEnum* ft);
void gui_comp_get_font_size(GUIComp* comp, i32* fs);

i32  gui_comp_num_children(GUIComp* comp);
i32  gui_comp_tex(GUIComp* comp);
bool gui_comp_is_text(GUIComp* comp);
bool gui_comp_is_hoverable(GUIComp* comp);
bool gui_comp_is_hovered(GUIComp* comp);
bool gui_comp_is_clickable(GUIComp* comp);
bool gui_comp_is_visible(GUIComp* comp);

#endif
