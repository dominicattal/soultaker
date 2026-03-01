#ifndef GUI_H
#define GUI_H

#include "game.h"
#include "gui.h"
#include "renderer.h"
#include "util.h"

void gui_init(void);
void gui_cleanup(void);
void gui_render(void);
f32  gui_get_dt(void);

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

typedef enum {
    GUI_PRESET_MAIN_MENU,
    GUI_PRESET_OPTIONS,
    GUI_PRESET_GAME,
    GUI_PRESET_RUNS,
    GUI_PRESET_TEST,
    NUM_GUI_PRESETS
} GUIPreset;

typedef enum {
    GUI_COMP_TYPING,
    GUI_COMP_BOSS_HEALTH,
    GUI_COMP_NOTIFICATIONS,
    GUI_COMP_INTERACTABLE,
    NUM_GUI_EVENT_COMPS,
    GUI_COMP_DEFAULT
} GUIEventCompEnum;

typedef enum {
    GUI_COMP_FLAG_HOVERABLE,
    GUI_COMP_FLAG_HOVERED,
    GUI_COMP_FLAG_CLICKABLE,
    GUI_COMP_FLAG_VISIBLE,
    GUI_COMP_FLAG_RELATIVE,
    GUI_COMP_FLAG_ALLOW_CHILD_CLICK,
    GUI_COMP_FLAG_ALLOW_OUT_OF_BOUNDS_CLICK,
    GUI_COMP_FLAG_POINT_TO_TEXT,
    GUI_COMP_FLAG_AUTO_FREE_DATA,
    GUI_COMP_FLAG_REVERSE_RENDER,
    GUI_COMP_FLAG_RENDER_CHILDREN_FIRST,

    // does not affect clickability, just if it is counted as found in the callback
    GUI_COMP_FLAG_IGNORE_MOUSE_BUTTON
} GUICompFlagEnum;

typedef struct GUIComp GUIComp;
typedef void (*GUIHoverFPtr)(GUIComp* comp, bool status); 
typedef void (*GUIClickFPtr)(GUIComp* comp, i32 button, i32 action, i32 mods);
typedef void (*GUIKeyFPtr)(GUIComp* comp, i32 key, i32 scancode, i32 action, i32 mods);
typedef void (*GUIUpdateFPtr)(GUIComp* comp, f32 dt);
typedef void (*GUIControlFPtr)(GUIComp* comp, ControlEnum ctrl, i32 action);
typedef void (*GUICompDestroyFPtr)(GUIComp* comp);

typedef struct GUIComp {
    GUIUpdateFPtr update;
    GUIHoverFPtr hover;
    GUIClickFPtr click;
    GUIKeyFPtr key;
    GUICompDestroyFPtr destroy;
    GUIControlFPtr control;
    GUIEventCompEnum event_id;
    void* data;
    char* name;
    char* text;
    GUIComp* parent;
    GUIComp** children;
    i32 num_children;
    FontEnum font;
    i32 x, y, w, h;
    i32 r, g, b, a;
    i32 tex;
    i32 halign;
    i32 valign;
    i32 text_halign;
    i32 text_valign;
    i32 text_length;
    i32 text_pos;
    i32 font_size;
    u32 flags;
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
    GUIComp* event_comps[NUM_GUI_EVENT_COMPS];
    void* event_comp_data[NUM_GUI_EVENT_COMPS];
    bool kill_thread;
    pthread_t thread_id;
    pthread_mutex_t data_mutex;
    f32 dt;
} GUIContext;

extern GUIContext gui_context;

GUIComp* gui_get_event_comp(GUIEventCompEnum type);
void gui_set_event_comp(GUIEventCompEnum type, GUIComp* comp);
bool gui_event_comp_equal(GUIEventCompEnum type, GUIComp* comp);

// events
void gui_create_boss_healthbar(char* name, Entity* boss_ptr);
void gui_update_boss_healthbar(Entity* boss);
void gui_destroy_boss_healthbar(Entity* boss);
void gui_create_notification(char* notif);
void gui_set_interactable(const char* desc, InteractableFuncPtr func_ptr, Map* map, MapNode* map_node);
void gui_refresh_inventory(void);

void gui_framebuffer_size_callback(i32 width, i32 height);
bool gui_cursor_pos_callback(f64 xpos, f64 ypos);
bool gui_key_callback(i32 key, i32 scancode, i32 action, i32 mods);
bool gui_mouse_button_callback(i32 button, i32 action, i32 mods);
bool gui_char_callback(u32 codepoint);
void gui_control_callback(ControlEnum ctrl, i32 action);

void gui_preset_load(GUIPreset preset);

// returns response from parsing command
char* gui_command_parse(char* command);

void gui_set_typing_comp(GUIComp* comp);

void gui_render_init(void);
void gui_update_vertex_data(void);
void gui_render_cleanup(void);

void align_comp_position_x(i32* position_x, u8 halign, i32 size_x, i32 x, i32 w);
void align_comp_position_y(i32* position_y, u8 valign, i32 size_y, i32 y, i32 h);

void gui_comp_hover(GUIComp* comp, bool status);
void gui_comp_click(GUIComp* comp, i32 button, i32 action, i32 mods);
void gui_comp_key(GUIComp* comp, i32 key, i32 scancode, i32 action, i32 mods);
void gui_comp_control(GUIComp* comp, ControlEnum ctrl, i32 action);
void gui_comp_update(GUIComp* comp, f32 dt);
void gui_update_comps_helper(GUIComp* comp, f32 dt);
void gui_update_comps(f32 dt);

// returns the x and y coordinates of the bottom left corner of the component in the screen
void gui_comp_get_true_position(GUIComp* comp, i32* x, i32* y);

// returns true if the mouse cursor is in the bounds of the comp by using
// gui_comp_get_true_position
bool gui_comp_contains_cursor(GUIComp* comp);

void gui_comp_init(void);
void gui_comp_cleanup(void);

// calculating the tight bounding box height to render comp text
i32 gui_comp_compute_text_height(GUIComp* comp);

// create a new gui component with default values
GUIComp* gui_comp_create(i16 x, i16 y, i16 w, i16 h);

void gui_comp_set_flag(GUIComp* comp, GUICompFlagEnum flag, bool val);
void gui_comp_toggle_flag(GUIComp* comp, GUICompFlagEnum flag);
bool gui_comp_get_flag(GUIComp* comp, GUICompFlagEnum flag);

// sets the name of the comp for use in gui_comp_get_by_name
void gui_comp_set_name(GUIComp* comp, const char* name);
GUIComp* gui_comp_get_by_name(const char* name);

// attach a component to another component. the child component's alignment 
// is done relative to its parent
void gui_comp_attach(GUIComp* parent, GUIComp* child);

// detach a child from its parent. if the parent does not own the child,
// then it will do nothing
void gui_comp_detach(GUIComp* parent, GUIComp* child);

// destroy component and its children. the component will call its destroy function
// before destroying its children. this is done so components that store its children
// as data can freely alter them without creating dangling pointers.
void gui_comp_destroy(GUIComp* comp);

// calls gui_comp_detach then gui_comp_destroy
void gui_comp_detach_and_destroy(GUIComp* parent, GUIComp* child);

// destroy all of a component's children. 
void gui_comp_destroy_children(GUIComp* comp);

// sets the comp's text and length. text becomes owned by the comp, so
// it should no longer be altered. text must be on the heap.
void gui_comp_set_text(GUIComp* comp, char* text);

// copies length characters from string text. text does not become owned by
// the comp, so it should be deallocated properly. text does not have to be on heap.
void gui_comp_copy_text(GUIComp* comp, const char* text);

// sets the comp's text pointer to text and does not free it when setting the text
// again or destroying the component
void gui_comp_point_to_text(GUIComp* comp, char* text);

// removes the comp's current text, freeing it and setting it to NULL
void gui_comp_remove_text(GUIComp* comp);

void gui_comp_insert_char(GUIComp* comp, const char character, i32 idx);
void gui_comp_delete_char(GUIComp* comp, i32 idx);
void gui_comp_update(GUIComp* comp, f32 dt);

void gui_comp_set_bbox(GUIComp* comp, i32 x, i32 y, i32 w, i32 h);
void gui_comp_set_color(GUIComp* comp, i32 r, i32 g, i32 b, i32 a);
void gui_comp_set_align(GUIComp* comp, i32 halign, i32 valign);
void gui_comp_set_text_align(GUIComp* comp, i32 halign, i32 valign);

void gui_comp_print(GUIComp* comp);

#endif
