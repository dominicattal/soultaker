#ifndef GUI_INTERNAL_H
#define GUI_INTERNAL_H

#include "../gui.h"
#include <semaphore.h>

typedef void (*GUIHoverFPtr)(GUIComp* comp, bool status); 
typedef void (*GUIClickFPtr)(GUIComp* comp, i32 button, i32 action, i32 mods);
typedef void (*GUIKeyFPtr)(GUIComp* comp, i32 key, i32 scancode, i32 action, i32 mods);
typedef void (*GUIUpdateFPtr)(GUIComp* comp, f32 dt);

typedef struct GUIComp {
    u64 info1;
    u64 info2;
    u64 info3;
    GUIUpdateFPtr update_func;
    GUIHoverFPtr hover_func;
    GUIClickFPtr click_func;
    GUIKeyFPtr key_func;
    void* data;
    GUIComp* parent;
    union {
        GUIComp** children;
        char* text;
    };
} GUIComp;

typedef struct GUIData {
    GLsizei instance_count;
    GLint length, capacity;
    GLfloat* buffer;
} GUIData;

typedef enum {
    GUI_COMP_TYPING,
    GUI_COMP_WEAPON_INFO,
    NUM_GUI_EVENT_COMPS
} GUIEventCompEnum;

typedef struct GUIContext {
    GUIData data;
    GUIData data_swap;
    GUIComp* root;
    GUIComp* event_comps[NUM_GUI_EVENT_COMPS];
    bool kill_thread;
    pthread_t thread_id;
    pthread_mutex_t data_mutex;
    f32 dt;
} GUIContext;

extern GUIContext gui_context;

GUIComp* gui_get_event_comp(GUIEventCompEnum type);
void gui_set_event_comp(GUIEventCompEnum type, GUIComp* comp);
bool gui_event_comp_equal(GUIEventCompEnum type, GUIComp* comp);

void gui_update_weapon_info(i32 weapon_id);

void gui_framebuffer_size_callback(i32 width, i32 height);
bool gui_cursor_pos_callback(f64 xpos, f64 ypos);
bool gui_key_callback(i32 key, i32 scancode, i32 action, i32 mods);
bool gui_mouse_button_callback(i32 button, i32 action, i32 mods);
bool gui_char_callback(u32 codepoint);

void gui_preset_load(GUIPreset preset);

const char* gui_parse_command(char* command);
void gui_set_typing_comp(GUIComp* comp);

void gui_render_init(void);
void gui_update_vertex_data(void);
void gui_render_cleanup(void);

void align_comp_position_x(i32* position_x, u8 halign, i32 size_x, i32 x, i32 w);
void align_comp_position_y(i32* position_y, u8 valign, i32 size_y, i32 y, i32 h);

void load_preset_test(GUIComp* root);
void load_preset_main_menu(GUIComp* root);
void load_preset_debug(GUIComp* root);

void gui_comp_hover(GUIComp* comp, bool status);
void gui_comp_click(GUIComp* comp, i32 button, i32 action, i32 mods);
void gui_comp_key(GUIComp* comp, i32 key, i32 scancode, i32 action, i32 mods);
void gui_comp_update(GUIComp* comp, f32 dt);

void gui_comp_init(void);
void gui_comp_cleanup(void);
GUIComp* gui_comp_create(i16 x, i16 y, i16 w, i16 h);
void gui_comp_attach(GUIComp* parent, GUIComp* child);
void gui_comp_detach(GUIComp* parent, GUIComp* child);
void gui_comp_destroy(GUIComp* comp);
void gui_comp_destroy_children(GUIComp* comp);
void gui_comp_detach_and_destroy(GUIComp* parent, GUIComp* child);
void gui_comp_set_text(GUIComp* comp, i32 length, const char* text);
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
void gui_comp_set_text_length(GUIComp* comp, i32 tl);
void gui_comp_set_text_pos(GUIComp* comp, i32 tp);
void gui_comp_inc_text_pos(GUIComp* comp);
void gui_comp_dec_text_pos(GUIComp* comp);

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
void gui_comp_get_text_length(GUIComp* comp, i32* tl);
void gui_comp_get_text_pos(GUIComp* comp, i32* tp);

i32  gui_comp_num_children(GUIComp* comp);
i32  gui_comp_tex(GUIComp* comp);
bool gui_comp_is_text(GUIComp* comp);
bool gui_comp_is_hoverable(GUIComp* comp);
bool gui_comp_is_hovered(GUIComp* comp);
bool gui_comp_is_clickable(GUIComp* comp);
bool gui_comp_is_visible(GUIComp* comp);
i32  gui_comp_text_length(GUIComp* comp);
char* gui_comp_text(GUIComp* comp);
i32  gui_comp_text_pos(GUIComp* comp);

#endif
