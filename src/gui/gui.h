#ifndef GUI_H
#define GUI_H

#include "../util/type.h"
#include "../util/vec.h"
#include "../util/indices.h"
#include "../window/window.h"
#include "../renderer/renderer.h"
#include "component/component.h"

typedef struct {
    u32 length, max_length;
    f32 *buffer;
    bool max_length_changed;
} GUI;

extern GUI gui;
extern Window window;

void gui_init(void);
void gui_update_data(void);
bool gui_update(void);
u32 gui_interact(i32 input);
void gui_mouse_button_callback(i32 button, i32 action);
void gui_key_callback(i32 key, i32 scancode, i32 action, i32 mods);
void gui_destroy(void);

#endif