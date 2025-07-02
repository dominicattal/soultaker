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

void gui_init(void);
void gui_cleanup(void);
void gui_render(void);
f32  gui_get_dt(void);

#endif
