#ifndef GUI_INTERNAL_H
#define GUI_INTERNAL_H

#include "../gui.h"


typedef struct GUIComp {
    u64 info1;
    u64 info2;
    void* update_func;
    void* hover_func;
    void* mouse_button_func;
    void* key_func;
    void* data;
    GUIComp* parent;
    union {
        GUIComp** children;
        char* text;
    };
} GUIComp;

typedef struct GUIData {
    GLint length, capacity;
    GLfloat* buffer;
} GUIData;

typedef struct GUIContext {
    GUIData data;
    GUIComp* root;
    bool kill_thread;
    pthread_t thread_id;
    pthread_mutex_t mutex;
} GUIContext;

extern GUIContext gui_context;

#endif
