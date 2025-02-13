#include "internal.h"
#include "../window.h"
#include <assert.h>
#include <string.h>

void gui_comp_init(void)
{ 
    gui_context.root = gui_comp_create(0, 0, window_resolution_x(), window_resolution_y());
    gui_comp_set_color(gui_context.root, 255, 0, 0, 255);

    GUIComp* base = gui_comp_create(30, 30, 700, 700);
    gui_comp_set_color(base, 255, 255, 255, 255);
    gui_comp_attach(gui_context.root, base);
    for (i32 i = 0; i < 16; i++) {
        GUIComp* comp = gui_comp_create(50, 50, 50, 50);
        gui_comp_set_color(comp, 0, 0, 255, 255);
        gui_comp_set_align(comp, i % 4, i / 4);
        gui_comp_attach(base, comp);
        GUIComp* comp2 = gui_comp_create(0, 0, 4, 4);
        gui_comp_set_align(comp2, ALIGN_CENTER, ALIGN_CENTER);
        gui_comp_set_color(comp2, 255, 255, 255, 255);
        gui_comp_attach(comp, comp2);
    }

    GUIComp* textbox = gui_comp_create(735, 30, 250, 700);
    gui_comp_set_is_text(textbox, true);
    gui_comp_set_text(textbox, "The quick brown fox jumps over the lazy dog");
    gui_comp_set_color(textbox, 255, 255, 255, 255);
    gui_comp_set_font_size(textbox, 16);
    gui_comp_set_font(textbox, FONT_MONOSPACE);
    gui_comp_attach(gui_context.root, textbox);
}

void gui_comp_cleanup(void)
{
    gui_comp_destroy(gui_context.root);
}

GUIComp* gui_comp_create(i16 x, i16 y, i16 w, i16 h)
{
    GUIComp* comp = calloc(1, sizeof(GUIComp));
    gui_comp_set_bbox(comp, x, y, w, h);
    gui_comp_set_tex(comp, TEX_COLOR);
    return comp;
} 

void gui_comp_attach(GUIComp* parent, GUIComp* child)
{
    i32 num_children;
    gui_comp_get_num_children(parent, &num_children);
    assert(num_children < MAX_NUM_CHILDREN);
    if (parent->children == NULL)
        parent->children = malloc(sizeof(GUIComp*));
    else
        parent->children = realloc(parent->children, (num_children + 1) * sizeof(GUIComp*));
    parent->children[num_children++] = child;
    child->parent = parent;
    gui_comp_set_num_children(parent, num_children);
}

void gui_comp_detach(GUIComp* parent, GUIComp* child)
{
    i32 num_children;
    gui_comp_get_num_children(parent, &num_children);
    for (i32 i = 0; i < num_children; i++) {
        if (parent->children[i] == child) {
            parent->children[i] = parent->children[--num_children];
            child->parent = NULL;
            if (num_children == 0) {
                free(parent->children);
                parent->children = NULL;
            } else {
                parent->children = realloc(parent->children, num_children * sizeof(GUIComp*));
            }
            gui_comp_set_num_children(parent, num_children);
            return;
        }
    }
}

void gui_comp_destroy(GUIComp* comp)
{
    for (int i = 0; i < gui_comp_num_children(comp); i++)
        gui_comp_destroy(comp->children[i]);
    free(comp->children);
    free(comp->data);
    free(comp);
}

void gui_comp_destroy_children(GUIComp* comp) {
    for (int i = 0; i < gui_comp_num_children(comp); i++)
        gui_comp_destroy(comp->children[i]);
    free(comp->children);
    gui_comp_set_num_children(comp, 0);
    comp->children = NULL;
}

void gui_comp_detach_and_destroy(GUIComp* parent, GUIComp* child)
{
    gui_comp_detach(parent, child);
    gui_comp_destroy(child);
}

void gui_comp_set_text(GUIComp* comp, const char* text)
{
    assert(gui_comp_is_text(comp));
    assert(text != NULL);
    u32 length;
    char* copied_text;
    free(comp->text);
    length = strlen(text);
    if (length == 0) {
        comp->text = NULL;
        return;
    }
    copied_text = malloc((length + 1) * sizeof(char));
    strncpy(copied_text, text, length + 1);
    comp->text = copied_text;
}

void gui_comp_insert_char(GUIComp* comp, const char c, i32 idx)
{
    assert(gui_comp_is_text(comp));
    u32 length = (comp->text == NULL) ? 0 : strlen(comp->text);
    char* new_text = malloc((length + 2) * sizeof(char));
    if (idx == -1 || (u32)idx >= length) {
        strncpy(new_text, comp->text, length);
        new_text[length] = c;
    } else {
        strncpy(new_text, comp->text, idx);
        new_text[idx] = c;
        strncpy(new_text, comp->text + idx + 1, length - idx + 1);
    }
    new_text[length+1] = '\0';
    free(comp->text);
    comp->text = new_text;
}

void gui_comp_delete_char(GUIComp* comp, i32 idx)
{
    assert(gui_comp_is_text(comp));
    if (comp->text == NULL) return;
    u32 length = strlen(comp->text);
    if (length == 1) {
        free(comp->text);
        comp->text = NULL;
        return;
    }
    char* new_text = malloc(length * sizeof(char));
    if (idx == -1 || (u32)idx >= length) {
        strncpy(new_text, comp->text, length-1);
    } else {
        strncpy(new_text, comp->text, idx);
        strncpy(new_text, comp->text + idx, length - idx + 1);
    }
    new_text[length-1] = '\0';
    free(comp->text);
    comp->text = new_text;
}

void gui_comp_hover(GUIComp* comp, bool status)
{

}

void gui_comp_click(GUIComp* comp, i32 button, i32 action)
{

}

void gui_comp_key(GUIComp* comp, i32 key, i32 scancode, i32 action, i32 mods)
{

}

void gui_comp_update(GUIComp* comp, f32 dt)
{

}

void gui_comp_add_data(GUIComp* comp, void* data)
{
}

void* gui_comp_remove_data(GUIComp* comp)
{
    return NULL;
}

// ---------------------------------------------------------------------------
// info1            | info2 (text)      | info2 (ele)
// 48 - x, y, w, h  | 2  - text_halign  | 8 - num_children
// 16 - tex         | 2  - text_valign  | 1 - update_children
// info2            | 10 - font_size    |
// 32 - rgba        | 4  - font         |
//  1 - is_text     |                   |
//  1 - hoverable   |                   |
//  1 - hovered     |                   |
//  1 - clickable   |                   |
//  1 - visible     |                   |
//  1 - relative    |                   |
//  2 - halign      |                   |
//  2 - valign      |                   |
// ---------------------------------------------------------------------------

// info1
#define X_SHIFT     0
#define X_BITS      12
#define Y_SHIFT     12
#define Y_BITS      12
#define W_SHIFT     24
#define W_BITS      12
#define H_SHIFT     36
#define H_BITS      12
#define TX_SHIFT    48
#define TX_BITS     16

// info2
#define R_SHIFT     0
#define R_BITS      8
#define G_SHIFT     8
#define G_BITS      8
#define B_SHIFT     16
#define B_BITS      8
#define A_SHIFT     24
#define A_BITS      8
#define IT_SHIFT    32
#define IT_BITS     1
#define HV_SHIFT    33
#define HV_BITS     1
#define HD_SHIFT    34
#define HD_BITS     1
#define CL_SHIFT    35
#define CL_BITS     1
#define VS_SHIFT    36
#define VS_BITS     1
#define RL_SHIFT    37
#define RL_BITS     1
#define HA_SHIFT    38
#define HA_BITS     2
#define VA_SHIFT    40
#define VA_BITS     2

// non text comp
#define NC_SHIFT    42
#define NC_BITS     8
#define UC_SHIFT    50
#define UC_BITS     1

// text comp
#define THA_SHIFT   42
#define THA_BITS    2
#define TVA_SHIFT   44
#define TVA_BITS    2
#define FS_SHIFT    46
#define FS_BITS     10
#define FT_SHIFT    56
#define FT_BITS     4

#define SMASK(BITS)         ((1<<BITS)-1)
#define GMASK(BITS, SHIFT)  ~((u64)SMASK(BITS)<<SHIFT)

// setters
void gui_comp_set_bbox(GUIComp* comp, i32 x, i32 y, i32 w, i32 h) {
    gui_comp_set_x(comp, x);
    gui_comp_set_y(comp, y);
    gui_comp_set_w(comp, w);
    gui_comp_set_h(comp, h);
}
void gui_comp_set_position(GUIComp* comp, i32 x, i32 y) {
    gui_comp_set_x(comp, x);
    gui_comp_set_y(comp, y);
}
void gui_comp_set_x(GUIComp* comp, i32 x) {
    comp->info1 = (comp->info1 & GMASK(X_BITS, X_SHIFT)) | ((u64)(x & SMASK(X_BITS)) << X_SHIFT);
}
void gui_comp_set_y(GUIComp* comp, i32 y) {
    comp->info1 = (comp->info1 & GMASK(Y_BITS, Y_SHIFT)) | ((u64)(y & SMASK(Y_BITS)) << Y_SHIFT);
}
void gui_comp_set_size(GUIComp* comp, i32 w, i32 h) {
    gui_comp_set_w(comp, w);
    gui_comp_set_h(comp, h);
}
void gui_comp_set_w(GUIComp* comp, i32 w) {
    comp->info1 = (comp->info1 & GMASK(W_BITS, W_SHIFT)) | ((u64)(w & SMASK(W_BITS)) << W_SHIFT);
}
void gui_comp_set_h(GUIComp* comp, i32 h) {
    comp->info1 = (comp->info1 & GMASK(H_BITS, H_SHIFT)) | ((u64)(h & SMASK(H_BITS)) << H_SHIFT);
}
void gui_comp_set_tex(GUIComp* comp, i32 tx) {
    comp->info1 = (comp->info1 & GMASK(TX_BITS, TX_SHIFT)) | ((u64)(tx & SMASK(TX_BITS)) << TX_SHIFT);
}
void gui_comp_set_color(GUIComp* comp, u8 r, u8 g, u8 b, u8 a) {
    gui_comp_set_r(comp, r);
    gui_comp_set_g(comp, g);
    gui_comp_set_b(comp, b);
    gui_comp_set_a(comp, a);
}
void gui_comp_set_r(GUIComp* comp, u8 r) {
    comp->info2 = (comp->info2 & GMASK(R_BITS, R_SHIFT)) | ((u64)(r & SMASK(R_BITS)) << R_SHIFT);
}
void gui_comp_set_g(GUIComp* comp, u8 g) {
    comp->info2 = (comp->info2 & GMASK(G_BITS, G_SHIFT)) | ((u64)(g & SMASK(G_BITS)) << G_SHIFT);
}
void gui_comp_set_b(GUIComp* comp, u8 b) {
    comp->info2 = (comp->info2 & GMASK(B_BITS, B_SHIFT)) | ((u64)(b & SMASK(B_BITS)) << B_SHIFT);
}
void gui_comp_set_a(GUIComp* comp, u8 a) {
    comp->info2 = (comp->info2 & GMASK(A_BITS, A_SHIFT)) | ((u64)(a & SMASK(A_BITS)) << A_SHIFT);
}
void gui_comp_set_is_text(GUIComp* comp, bool it) {
    comp->info2 = (comp->info2 & GMASK(IT_BITS, IT_SHIFT)) | ((u64)(it & SMASK(IT_BITS)) << IT_SHIFT);
}
void gui_comp_set_hoverable(GUIComp* comp, bool hv) {
    comp->info2 = (comp->info2 & GMASK(HV_BITS, HV_SHIFT)) | ((u64)(hv & SMASK(HV_BITS)) << HV_SHIFT);
}
void gui_comp_set_hovered(GUIComp* comp, bool hd) {
    comp->info2 = (comp->info2 & GMASK(HD_BITS, HD_SHIFT)) | ((u64)(hd & SMASK(HD_BITS)) << HD_SHIFT);
}
void gui_comp_set_clickable(GUIComp* comp, bool cl) {
    comp->info2 = (comp->info2 & GMASK(CL_BITS, CL_SHIFT)) | ((u64)(cl & SMASK(CL_BITS)) << CL_SHIFT);
}
void gui_comp_set_visible(GUIComp* comp, bool vs) {
    comp->info2 = (comp->info2 & GMASK(VS_BITS, VS_SHIFT)) | ((u64)(vs & SMASK(VS_BITS)) << VS_SHIFT);
}
void gui_comp_set_relative(GUIComp* comp, bool rl) {
    comp->info2 = (comp->info2 & GMASK(RL_BITS, RL_SHIFT)) | ((u64)(rl & SMASK(RL_BITS)) << RL_SHIFT);
}
void gui_comp_set_align(GUIComp* comp, u8 ha, u8 va) {
    gui_comp_set_halign(comp, ha);
    gui_comp_set_valign(comp, va);
}
void gui_comp_set_halign(GUIComp* comp, u8 ha) {
    comp->info2 = (comp->info2 & GMASK(HA_BITS, HA_SHIFT)) | ((u64)(ha & SMASK(HA_BITS)) << HA_SHIFT);
}
void gui_comp_set_valign(GUIComp* comp, u8 va) {
    comp->info2 = (comp->info2 & GMASK(VA_BITS, VA_SHIFT)) | ((u64)(va & SMASK(VA_BITS)) << VA_SHIFT);
}
void gui_comp_set_num_children(GUIComp* comp, i32 nc) {
    comp->info2 = (comp->info2 & GMASK(NC_BITS, NC_SHIFT)) | ((u64)(nc & SMASK(NC_BITS)) << NC_SHIFT);
}
void gui_comp_set_text_align(GUIComp* comp, u8 tha, u8 tva) {
    gui_comp_set_text_halign(comp, tha);
    gui_comp_set_text_valign(comp, tva);
}
void gui_comp_set_text_halign(GUIComp* comp, u8 tha) {
    comp->info2 = (comp->info2 & GMASK(THA_BITS, THA_SHIFT)) | ((u64)(tha & SMASK(THA_BITS)) << THA_SHIFT);
}
void gui_comp_set_text_valign(GUIComp* comp, u8 tva) {
    comp->info2 = (comp->info2 & GMASK(TVA_BITS, TVA_SHIFT)) | ((u64)(tva & SMASK(TVA_BITS)) << TVA_SHIFT);
}
void gui_comp_set_font(GUIComp* comp, FontEnum ft) {
    comp->info2 = (comp->info2 & GMASK(FT_BITS, FT_SHIFT)) | ((u64)(ft & SMASK(FT_BITS)) << FT_SHIFT);
}
void gui_comp_set_font_size(GUIComp* comp, i32 fs) {
    comp->info2 = (comp->info2 & GMASK(FS_BITS, FS_SHIFT)) | ((u64)(fs & SMASK(FS_BITS)) << FS_SHIFT);
}

// getters 1
void gui_comp_get_bbox(GUIComp* comp, i32* x, i32* y, i32* w, i32* h) {
    gui_comp_get_x(comp, x);
    gui_comp_get_y(comp, y);
    gui_comp_get_w(comp, w);
    gui_comp_get_h(comp, h);
}
void gui_comp_get_position(GUIComp* comp, i32* x, i32* y) {
    gui_comp_get_x(comp, x);
    gui_comp_get_y(comp, y);
}
void gui_comp_get_x(GUIComp* comp, i32* x) {
    *x = (comp->info1 >> X_SHIFT) & SMASK(X_BITS);
}
void gui_comp_get_y(GUIComp* comp, i32* y) {
    *y = (comp->info1 >> Y_SHIFT) & SMASK(Y_BITS);
}
void gui_comp_get_size(GUIComp* comp, i32* w, i32* h) {
    gui_comp_get_w(comp, w);
    gui_comp_get_h(comp, h);
}
void gui_comp_get_w(GUIComp* comp, i32* w) {
    *w = (comp->info1 >> W_SHIFT) & SMASK(W_BITS);
}
void gui_comp_get_h(GUIComp* comp, i32* h) {
    *h = (comp->info1 >> H_SHIFT) & SMASK(H_BITS);
}
void gui_comp_get_tex(GUIComp* comp, i32* tx) {
    *tx = (comp->info1 >> TX_SHIFT) & SMASK(TX_BITS);
}
void gui_comp_get_color(GUIComp* comp, u8* r, u8* g, u8* b, u8* a) {
    gui_comp_get_r(comp, r);
    gui_comp_get_g(comp, g);
    gui_comp_get_b(comp, b);
    gui_comp_get_a(comp, a);
}
void gui_comp_get_r(GUIComp* comp, u8* r) {
    *r = (comp->info2 >> R_SHIFT) & SMASK(R_BITS);
}
void gui_comp_get_g(GUIComp* comp, u8* g) {
    *g = (comp->info2 >> G_SHIFT) & SMASK(G_BITS);
}
void gui_comp_get_b(GUIComp* comp, u8* b) {
    *b = (comp->info2 >> B_SHIFT) & SMASK(B_BITS);
}
void gui_comp_get_a(GUIComp* comp, u8* a) {
    *a = (comp->info2 >> A_SHIFT) & SMASK(A_BITS);
}
void gui_comp_get_is_text(GUIComp* comp, bool* it) {
    *it = (comp->info2 >> IT_SHIFT) & SMASK(IT_BITS);
}
void gui_comp_get_hoverable(GUIComp* comp, bool* hv) {
    *hv = (comp->info2 >> HV_SHIFT) & SMASK(HV_BITS);
}
void gui_comp_get_hovered(GUIComp* comp, bool* hd) {
    *hd = (comp->info2 >> HD_SHIFT) & SMASK(HD_BITS);
}
void gui_comp_get_clickable(GUIComp* comp, bool* cl) {
    *cl = (comp->info2 >> CL_SHIFT) & SMASK(CL_BITS);
}
void gui_comp_get_visible(GUIComp* comp, bool* vs) {
    *vs = (comp->info2 >> VS_SHIFT) & SMASK(VS_BITS);
}
void gui_comp_get_relative(GUIComp* comp, bool* rl) {
    *rl = (comp->info2 >> RL_SHIFT) & SMASK(RL_BITS);
}
void gui_comp_get_align(GUIComp* comp, u8* ha, u8* va) {
    gui_comp_get_halign(comp, ha);
    gui_comp_get_valign(comp, va);
}
void gui_comp_get_halign(GUIComp* comp, u8* ha) {
    *ha = (comp->info2 >> HA_SHIFT) & SMASK(HA_BITS);
}
void gui_comp_get_valign(GUIComp* comp, u8* va) {
    *va = (comp->info2 >> VA_SHIFT) & SMASK(VA_BITS);
}
void gui_comp_get_num_children(GUIComp* comp, i32* nc) {
    *nc = (comp->info2 >> NC_SHIFT) & SMASK(NC_BITS);
}
void gui_comp_get_text_align(GUIComp* comp, u8* tha, u8* tva) {
    gui_comp_get_text_halign(comp, tha);
    gui_comp_get_text_valign(comp, tva);
}
void gui_comp_get_text_halign(GUIComp* comp, u8* tha) {
    *tha = (comp->info2 >> THA_SHIFT) & SMASK(THA_BITS);
}
void gui_comp_get_text_valign(GUIComp* comp, u8* tva) {
    *tva = (comp->info2 >> TVA_SHIFT) & SMASK(TVA_BITS);
}
void gui_comp_get_font(GUIComp* comp, FontEnum* ft) {
    *ft = (comp->info2 >> FT_SHIFT) & SMASK(FT_BITS);
}
void gui_comp_get_font_size(GUIComp* comp, i32* fs) {
    *fs = (comp->info2 >> FS_SHIFT) & SMASK(FS_BITS);
}

// getters 2
i32 gui_comp_num_children(GUIComp* comp) {
    if (gui_comp_is_text(comp))
        return 0;
    return (comp->info2 >> NC_SHIFT) & SMASK(NC_BITS);
}
i32  gui_comp_tex(GUIComp* comp){
    return (comp->info1 >> TX_SHIFT) & SMASK(TX_BITS);
}
bool gui_comp_is_text(GUIComp* comp) {
    return (comp->info2 >> IT_SHIFT) & SMASK(IT_BITS);
}
bool gui_comp_is_hoverable(GUIComp* comp) {
    return (comp->info2 >> HV_SHIFT) & SMASK(HV_BITS);
}
bool gui_comp_is_hovered(GUIComp* comp) {
    return (comp->info2 >> HD_SHIFT) & SMASK(HD_BITS);
}
bool gui_comp_is_clickable(GUIComp* comp) {
    return (comp->info2 >> CL_SHIFT) & SMASK(CL_BITS);
}
bool gui_comp_is_visible(GUIComp* comp) {
    return (comp->info2 >> VS_SHIFT) & SMASK(VS_BITS);
}
