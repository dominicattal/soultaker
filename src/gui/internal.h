#ifndef GUI_INTERNAL_H
#define GUI_INTERNAL_H

#include "../gui.h"
#include <semaphore.h>

#define EVENT_QUEUE_LENGTH 1024

typedef enum {
    GUI_EVENT_NONE,
    GUI_EVENT_CURSOR_POS_CALLBACK,
    GUI_EVENT_MOUSE_BUTTON_CALLBACK,
    GUI_EVENT_KEY_CALLBACK,
    GUI_EVENT_ATTACH,
    GUI_EVENT_DETACH,
    GUI_EVENT_DESTROY,
    GUI_EVENT_DESTROY_CHILDREN,
    GUI_EVENT_DETACH_AND_DESTROY,
    GUI_EVENT_SET_TEXT,
    GUI_EVENT_HOVER,
    GUI_EVENT_CLICK,
    GUI_EVENT_KEY,
    GUI_EVENT_UPDATE,
    GUI_EVENT_ADD_DATA,
    GUI_EVENT_REMOVE_DATA
} GUIEventEnum;

typedef struct GUIEvent {
    GUIEventEnum type;
    struct {
        f32 xpos;
        f32 ypos;
        i32 button;
        i32 action;
        i32 mods;
    } args;
} GUIEvent;

typedef struct GUIEventQueue {
    GUIEvent buffer[EVENT_QUEUE_LENGTH];
    sem_t mutex, empty, full;
    i32 head, tail, count;
} GUIEventQueue;

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
    pthread_mutex_t data_mutex;
    GUIEventQueue event_queue;
    f32 dt;
} GUIContext;

extern GUIContext gui_context;

bool gui_cursor_pos_callback_internal(f64 xpos, f64 ypos);

void gui_event_queue_init(GUIEventQueue* queue);
void gui_event_enqueue(GUIEventQueue* queue, GUIEvent event);
GUIEvent gui_event_dequeue(GUIEventQueue* queue);
void gui_event_queue_flush(GUIEventQueue* queue);
void gui_event_queue_cleanup(GUIEventQueue* queue);

void align_comp_position_x(i32* position_x, u8 halign, i32 size_x, i32 x, i32 w);
void align_comp_position_y(i32* position_y, u8 valign, i32 size_y, i32 y, i32 h);

void load_preset_test(GUIComp* root);

#endif
