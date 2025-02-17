#ifndef GUI_INTERNAL_H
#define GUI_INTERNAL_H

#include "../gui.h"

typedef struct GUIComp {
    u64 info1;
    u64 info2;
    void* update_func;
    GUIHoverFPtr hover_func;
    void* click_func;
    void* key_func;
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

typedef struct GUIContext {
    GUIData data;
    GUIData data_swap;
    GUIComp* root;
    bool kill_thread;
    pthread_t thread_id;
    pthread_mutex_t mutex;
} GUIContext;

extern GUIContext gui_context;

void align_comp_position_x(i32* position_x, u8 halign, i32 size_x, i32 x, i32 w);
void align_comp_position_y(i32* position_y, u8 valign, i32 size_y, i32 y, i32 h);

void load_preset_test(GUIComp* root);

#endif
