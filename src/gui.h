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

typedef struct GUIComp GUIComp;

typedef enum {
    GUI_PRESET_TEST,
    GUI_PRESET_MAIN_MENU,
    GUI_PRESET_DEBUG,
    NUM_GUI_PRESETS
} GUIPreset;

typedef void (*GUIHoverFPtr)(GUIComp* comp, bool status); 
typedef void (*GUIClickFPtr)(GUIComp* comp, i32 button, i32 action, i32 mods);
typedef void (*GUIKeyFPtr)(GUIComp* comp, i32 key, i32 scancode, i32 action, i32 mods);
typedef void (*GUIUpdateFPtr)(GUIComp* comp, f32 dt);

void gui_preset_load(GUIPreset preset);

void gui_init(void);
void gui_cleanup(void);
void gui_render(void);
f32  gui_dt(void);

void gui_framebuffer_size_callback(i32 width, i32 height);
bool gui_cursor_pos_callback(f64 xpos, f64 ypos);
bool gui_key_callback(i32 key, i32 scancode, i32 action, i32 mods);
bool gui_mouse_button_callback(i32 button, i32 action, i32 mods);

#endif
