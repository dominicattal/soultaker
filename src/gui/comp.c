#include "internal.h"

void gui_comp_init(void)
{
}

void gui_comp_cleanup(void)
{
}



#if 0
#include "../gui.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

#define NUM_COMPONENT_FUNCS 6

#define COMP_FUNC_INIT      0
#define COMP_FUNC_HOVER     1
#define COMP_FUNC_CLICK     2
#define COMP_FUNC_KEY       3
#define COMP_FUNC_UPDATE    4
#define COMP_FUNC_DESTROY   5

static void (*component_functions[NUM_COMPONENTS][NUM_COMPONENT_FUNCS])();

static void initialize_functions(void);

void gui_comp_init(void)
{
    initialize_functions();
}

void gui_comp_cleanup(void)
{
}

Component* gui_comp_create(i16 x, i16 y, i16 w, i16 h, CompID id)
{
    Component* comp = malloc(sizeof(Component));
    comp->info1 = comp->info2 = 0;
    comp_set_bbox(comp, x, y, w, h);
    comp_set_color(comp, 255, 255, 255, 255);
    comp_set_id(comp, id);
    comp_set_visible(comp, TRUE);
    comp_set_tex(comp, TEX_COLOR);
    comp->children = NULL;
    comp->data = NULL;
    component_functions[comp_id(comp)][COMP_FUNC_INIT](comp);
    return comp;
}

void gui_comp_attach(Component* parent, Component* child)
{
    i32 num_children;
    comp_get_num_children(parent, &num_children);
    assert(num_children < MAX_NUM_CHILDREN);
    if (parent->children == NULL)
        parent->children = malloc(sizeof(Component*));
    else
        parent->children = realloc(parent->children, (num_children + 1) * sizeof(Component*));
    parent->children[num_children++] = child;
    comp_set_num_children(parent, num_children);
}

void gui_comp_detach(Component* parent, Component* child)
{
    i32 num_children;
    comp_get_num_children(parent, &num_children);
    for (i32 i = 0; i < num_children; i++) {
        if (parent->children[i] == child) {
            parent->children[i] = parent->children[--num_children];
            if (num_children == 0) {
                free(parent->children);
                parent->children = NULL;
            } else {
                parent->children = realloc(parent->children, num_children * sizeof(Component*));
            }
            comp_set_num_children(parent, num_children);
            return;
        }
    }
}

void gui_comp_destroy(Component* comp)
{
    component_functions[comp_id(comp)][COMP_FUNC_HOVER](comp);
    for (int i = 0; i < comp_num_children(comp); i++)
        comp_destroy(comp->children[i]);
    free(comp->children);
    free(comp->data);
    free(comp);
}

void gui_comp_destroy_children(Component* comp) {
    for (int i = 0; i < comp_num_children(comp); i++)
        comp_destroy(comp->children[i]);
    free(comp->children);
    comp_set_num_children(comp, 0);
    comp->children = NULL;
}

void gui_comp_detach_and_destroy(Component* parent, Component* child)
{
    comp_detach(parent, child);
    comp_destroy(child);
}

void gui_comp_set_text(Component* comp, const char* text)
{
    assert(comp_is_text(comp));
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

void gui_comp_insert_char(Component* comp, const char c, i32 idx)
{
    assert(comp_is_text(comp));
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

void gui_comp_delete_char(Component* comp, i32 idx)
{
    assert(comp_is_text(comp));
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

void gui_comp_hover(Component* comp, bool status)
{
    component_functions[comp_id(comp)][COMP_FUNC_HOVER](comp, status);
}

void gui_comp_click(Component* comp, i32 button, i32 action)
{
    component_functions[comp_id(comp)][COMP_FUNC_CLICK](comp, button, action);
}

void gui_comp_key(Component* comp, i32 key, i32 scancode, i32 action, i32 mods)
{
    component_functions[comp_id(comp)][COMP_FUNC_KEY](comp, key, scancode, action, mods);
}

void gui_comp_update(Component* comp, f64 dt)
{
    component_functions[comp_id(comp)][COMP_FUNC_UPDATE](comp, dt);
}

/* --------------------------------- */

#define void_func_ptr void (*)()

static void do_nothing() {}

static void initialize_functions(void)
{
    for (i32 i = 0; i < NUM_COMPONENTS; i++) 
        for (i32 j = 0; j < NUM_COMPONENT_FUNCS; j++)
            component_functions[i][j] = do_nothing;
    
    component_functions[COMP_DEFAULT][COMP_FUNC_INIT]       = (void_func_ptr)comp_default_init;
    component_functions[COMP_TEXTBOX][COMP_FUNC_INIT]       = (void_func_ptr)comp_textbox_init;
    component_functions[COMP_TEXTBOX][COMP_FUNC_HOVER]      = (void_func_ptr)comp_textbox_hover;
    component_functions[COMP_TEXTBOX][COMP_FUNC_CLICK]      = (void_func_ptr)comp_textbox_click;
    component_functions[COMP_TEXTBOX][COMP_FUNC_KEY]        = (void_func_ptr)comp_textbox_key;
    component_functions[COMP_TEXTBOX][COMP_FUNC_UPDATE]     = (void_func_ptr)comp_textbox_update;
    component_functions[COMP_TEXTBOX][COMP_FUNC_DESTROY]    = (void_func_ptr)comp_textbox_destroy;
    component_functions[COMP_DEBUG][COMP_FUNC_INIT]         = (void_func_ptr)comp_debug_init;
    component_functions[COMP_DEBUG][COMP_FUNC_KEY]          = (void_func_ptr)comp_debug_key;
    component_functions[COMP_DEBUG][COMP_FUNC_UPDATE]       = (void_func_ptr)comp_debug_update;
    component_functions[COMP_BUTTON][COMP_FUNC_INIT]        = (void_func_ptr)comp_button_init;
    component_functions[COMP_BUTTON][COMP_FUNC_HOVER]       = (void_func_ptr)comp_button_hover;
    component_functions[COMP_BUTTON][COMP_FUNC_CLICK]       = (void_func_ptr)comp_button_click;
    component_functions[COMP_BUTTON][COMP_FUNC_DESTROY]     = (void_func_ptr)comp_button_destroy;
    component_functions[COMP_SLIDESHOW][COMP_FUNC_INIT]     = (void_func_ptr)comp_slideshow_init;
    component_functions[COMP_SLIDESHOW][COMP_FUNC_UPDATE]   = (void_func_ptr)comp_slideshow_update;
    component_functions[COMP_SLIDESHOW][COMP_FUNC_KEY]      = (void_func_ptr)comp_slideshow_key;
    component_functions[COMP_SLIDESHOW][COMP_FUNC_DESTROY]  = (void_func_ptr)comp_slideshow_destroy;
    component_functions[COMP_DEATH][COMP_FUNC_INIT]         = (void_func_ptr)comp_death_init;
    component_functions[COMP_DEATH][COMP_FUNC_UPDATE]       = (void_func_ptr)comp_death_update;
}

// ---------------------------------------------------------------------------
// info1            | info2 (same)   | info2 (text)      | info2 (ele)
//  7 - id          | 24 - w, h      | 2  - halign       | 8 - num_children
//  1 - is_text     | 16 - tex       | 2  - valign       | 1 - update_children
// 32 - r, g, b, a  |  1 - hoverable | 10 - font_size    |
// 24 - x, y        |  1 - hovered   | 4  - font         |
//                  |  1 - clickable |                   |
//                  |  1 - visible   |                   |
// ---------------------------------------------------------------------------

#define ID_SHIFT    0
#define ID_BITS     7
#define IT_SHIFT    7
#define IT_BITS     1
#define R_SHIFT     8
#define R_BITS      8
#define G_SHIFT     16
#define G_BITS      8
#define B_SHIFT     24
#define B_BITS      8
#define A_SHIFT     32
#define A_BITS      8
#define X_SHIFT     40
#define X_BITS      12
#define Y_SHIFT     52
#define Y_BITS      12

#define W_SHIFT     0
#define W_BITS      12
#define H_SHIFT     12
#define H_BITS      12
#define TX_SHIFT    24
#define TX_BITS     16
#define HV_SHIFT    40
#define HV_BITS     1
#define HD_SHIFT    41
#define HD_BITS     1
#define CL_SHIFT    42
#define CL_BITS     1
#define VS_SHIFT    43
#define VS_BITS     1

#define NC_SHIFT    44
#define NC_BITS     8

#define HA_SHIFT    44
#define HA_BITS     2
#define VA_SHIFT    46
#define VA_BITS     2
#define FS_SHIFT    48
#define FS_BITS     10
#define FT_SHIFT    58
#define FT_BITS     4

#define SMASK(BITS)         ((1<<BITS)-1)
#define GMASK(BITS, SHIFT)  ~((u64)SMASK(BITS)<<SHIFT)

// setters
void gui_comp_set_id(Component* comp, CompID id) {
    comp->info1 = (comp->info1 & GMASK(ID_BITS, ID_SHIFT)) | ((u64)(id & SMASK(ID_BITS)) << ID_SHIFT);
}
void gui_comp_set_is_text(Component* comp, bool it) {
    comp->info1 = (comp->info1 & GMASK(IT_BITS, IT_SHIFT)) | ((u64)(it & SMASK(IT_BITS)) << IT_SHIFT);
}
void gui_comp_set_color(Component* comp, u8 r, u8 g, u8 b, u8 a) {
    comp_set_r(comp, r);
    comp_set_g(comp, g);
    comp_set_b(comp, b);
    comp_set_a(comp, a);
}
void gui_comp_set_r(Component* comp, u8 r) {
    comp->info1 = (comp->info1 & GMASK(R_BITS, R_SHIFT)) | ((u64)(r & SMASK(R_BITS)) << R_SHIFT);
}
void gui_comp_set_g(Component* comp, u8 g) {
    comp->info1 = (comp->info1 & GMASK(G_BITS, G_SHIFT)) | ((u64)(g & SMASK(G_BITS)) << G_SHIFT);
}
void gui_comp_set_b(Component* comp, u8 b) {
    comp->info1 = (comp->info1 & GMASK(B_BITS, B_SHIFT)) | ((u64)(b & SMASK(B_BITS)) << B_SHIFT);
}
void gui_comp_set_a(Component* comp, u8 a) {
    comp->info1 = (comp->info1 & GMASK(A_BITS, A_SHIFT)) | ((u64)(a & SMASK(A_BITS)) << A_SHIFT);
}
void gui_comp_set_bbox(Component* comp, i32 x, i32 y, i32 w, i32 h) {
    comp_set_x(comp, x);
    comp_set_y(comp, y);
    comp_set_w(comp, w);
    comp_set_h(comp, h);
}
void gui_comp_set_position(Component* comp, i32 x, i32 y) {
    comp_set_x(comp, x);
    comp_set_y(comp, y);
}
void gui_comp_set_x(Component* comp, i32 x) {
    comp->info1 = (comp->info1 & GMASK(X_BITS, X_SHIFT)) | ((u64)(x & SMASK(X_BITS)) << X_SHIFT);
}
void gui_comp_set_y(Component* comp, i32 y) {
    comp->info1 = (comp->info1 & GMASK(Y_BITS, Y_SHIFT)) | ((u64)(y & SMASK(Y_BITS)) << Y_SHIFT);
}
void gui_comp_set_size(Component* comp, i32 w, i32 h) {
    comp_set_w(comp, w);
    comp_set_h(comp, h);
}
void gui_comp_set_w(Component* comp, i32 w) {
    comp->info2 = (comp->info2 & GMASK(W_BITS, W_SHIFT)) | ((u64)(w & SMASK(W_BITS)) << W_SHIFT);
}
void gui_comp_set_h(Component* comp, i32 h) {
    comp->info2 = (comp->info2 & GMASK(H_BITS, H_SHIFT)) | ((u64)(h & SMASK(H_BITS)) << H_SHIFT);
}
void gui_comp_set_num_children(Component* comp, i32 nc) {
    comp->info2 = (comp->info2 & GMASK(NC_BITS, NC_SHIFT)) | ((u64)(nc & SMASK(NC_BITS)) << NC_SHIFT);
}
void gui_comp_set_align(Component* comp, u8 ha, u8 va) {
    comp_set_halign(comp, ha);
    comp_set_valign(comp, va);
}
void gui_comp_set_halign(Component* comp, u8 ha) {
    comp->info2 = (comp->info2 & GMASK(HA_BITS, HA_SHIFT)) | ((u64)(ha & SMASK(HA_BITS)) << HA_SHIFT);
}
void gui_comp_set_valign(Component* comp, u8 va) {
    comp->info2 = (comp->info2 & GMASK(VA_BITS, VA_SHIFT)) | ((u64)(va & SMASK(VA_BITS)) << VA_SHIFT);
}
void gui_comp_set_hoverable(Component* comp, bool hv) {
    comp->info2 = (comp->info2 & GMASK(HV_BITS, HV_SHIFT)) | ((u64)(hv & SMASK(HV_BITS)) << HV_SHIFT);
}
void gui_comp_set_hovered(Component* comp, bool hd) {
    comp->info2 = (comp->info2 & GMASK(HD_BITS, HD_SHIFT)) | ((u64)(hd & SMASK(HD_BITS)) << HD_SHIFT);
}
void gui_comp_set_clickable(Component* comp, bool cl) {
    comp->info2 = (comp->info2 & GMASK(CL_BITS, CL_SHIFT)) | ((u64)(cl & SMASK(CL_BITS)) << CL_SHIFT);
}
void gui_comp_set_visible(Component* comp, bool vs) {
    comp->info2 = (comp->info2 & GMASK(VS_BITS, VS_SHIFT)) | ((u64)(vs & SMASK(VS_BITS)) << VS_SHIFT);
}
void gui_comp_set_tex(Component* comp, i32 tx) {
    comp->info2 = (comp->info2 & GMASK(TX_BITS, TX_SHIFT)) | ((u64)(tx & SMASK(TX_BITS)) << TX_SHIFT);
}
void gui_comp_set_font(Component* comp, Font ft) {
    comp->info2 = (comp->info2 & GMASK(FT_BITS, FT_SHIFT)) | ((u64)(ft & SMASK(FT_BITS)) << FT_SHIFT);
}
void gui_comp_set_font_size(Component* comp, i32 fs) {
    comp->info2 = (comp->info2 & GMASK(FS_BITS, FS_SHIFT)) | ((u64)(fs & SMASK(FS_BITS)) << FS_SHIFT);
}

// getters 1
void gui_comp_get_id(Component* comp, CompID* id) {
    *id = (comp->info1 >> ID_SHIFT) & SMASK(ID_BITS);
}
void gui_comp_get_is_text(Component* comp, bool* it) {
    *it = (comp->info1 >> IT_SHIFT) & SMASK(IT_BITS);
}
void gui_comp_get_color(Component* comp, u8* r, u8* g, u8* b, u8* a) {
    comp_get_r(comp, r);
    comp_get_g(comp, g);
    comp_get_b(comp, b);
    comp_get_a(comp, a);
}
void gui_comp_get_r(Component* comp, u8* r) {
    *r = (comp->info1 >> R_SHIFT) & SMASK(R_BITS);
}
void gui_comp_get_g(Component* comp, u8* g) {
    *g = (comp->info1 >> G_SHIFT) & SMASK(G_BITS);
}
void gui_comp_get_b(Component* comp, u8* b) {
    *b = (comp->info1 >> B_SHIFT) & SMASK(B_BITS);
}
void gui_comp_get_a(Component* comp, u8* a) {
    *a = (comp->info1 >> A_SHIFT) & SMASK(A_BITS);
}
void gui_comp_get_bbox(Component* comp, i32* x, i32* y, i32* w, i32* h) {
    comp_get_x(comp, x);
    comp_get_y(comp, y);
    comp_get_w(comp, w);
    comp_get_h(comp, h);
}
void gui_comp_get_position(Component* comp, i32* x, i32* y) {
    comp_get_x(comp, x);
    comp_get_y(comp, y);
}
void gui_comp_get_x(Component* comp, i32* x) {
    *x = (comp->info1 >> X_SHIFT) & SMASK(X_BITS);
}
void gui_comp_get_y(Component* comp, i32* y) {
    *y = (comp->info1 >> Y_SHIFT) & SMASK(Y_BITS);
}
void gui_comp_get_size(Component* comp, i32* w, i32* h) {
    comp_get_w(comp, w);
    comp_get_h(comp, h);
}
void gui_comp_get_w(Component* comp, i32* w) {
    *w = (comp->info2 >> W_SHIFT) & SMASK(W_BITS);
}
void gui_comp_get_h(Component* comp, i32* h) {
    *h = (comp->info2 >> H_SHIFT) & SMASK(H_BITS);
}
void gui_comp_get_num_children(Component* comp, i32* nc) {
    *nc = (comp->info2 >> NC_SHIFT) & SMASK(NC_BITS);
}
void gui_comp_get_align(Component* comp, u8* ha, u8* va) {
    comp_get_halign(comp, ha);
    comp_get_valign(comp, va);
}
void gui_comp_get_halign(Component* comp, u8* ha) {
    *ha = (comp->info2 >> HA_SHIFT) & SMASK(HA_BITS);
}
void gui_comp_get_valign(Component* comp, u8* va) {
    *va = (comp->info2 >> VA_SHIFT) & SMASK(VA_BITS);
}
void gui_comp_get_hoverable(Component* comp, bool* hv) {
    *hv = (comp->info2 >> HV_SHIFT) & SMASK(HV_BITS);
}
void gui_comp_get_hovered(Component* comp, bool* hd) {
    *hd = (comp->info2 >> HD_SHIFT) & SMASK(HD_BITS);
}
void gui_comp_get_clickable(Component* comp, bool* cl) {
    *cl = (comp->info2 >> CL_SHIFT) & SMASK(CL_BITS);
}
void gui_comp_get_visible(Component* comp, bool* vs) {
    *vs = (comp->info2 >> VS_SHIFT) & SMASK(VS_BITS);
}
void gui_comp_get_tex(Component* comp, i32* tx) {
    *tx = (comp->info2 >> TX_SHIFT) & SMASK(TX_BITS);
}
void gui_comp_get_font(Component* comp, Font* ft) {
    *ft = (comp->info2 >> FT_SHIFT) & SMASK(FT_BITS);
}
void gui_comp_get_font_size(Component* comp, i32* fs) {
    *fs = (comp->info2 >> FS_SHIFT) & SMASK(FS_BITS);
}

// getters 2
CompID comp_id(Component* comp) {
    return (comp->info1 >> ID_SHIFT) & SMASK(ID_BITS);
}
i32 comp_num_children(Component* comp) {
    if (comp_is_text(comp))
        return 0;
    return (comp->info2 >> NC_SHIFT) & SMASK(NC_BITS);
}
i32  comp_tex(Component* comp){
    return (comp->info2 >> TX_SHIFT) & SMASK(TX_BITS);
}
bool comp_is_text(Component* comp) {
    return (comp->info1 >> IT_SHIFT) & SMASK(IT_BITS);
}
bool comp_is_hoverable(Component* comp) {
    return (comp->info2 >> HV_SHIFT) & SMASK(HV_BITS);
}
bool comp_is_hovered(Component* comp) {
    return (comp->info2 >> HD_SHIFT) & SMASK(HD_BITS);
}
bool comp_is_clickable(Component* comp) {
    return (comp->info2 >> CL_SHIFT) & SMASK(CL_BITS);
}
bool comp_is_visible(Component* comp) {
    return (comp->info2 >> VS_SHIFT) & SMASK(VS_BITS);
}
#endif
