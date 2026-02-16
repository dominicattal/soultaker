#include "event.h"
#include "game.h"
#include "gui.h"

#define EVENT_QUEUE_CAPACITY 32

typedef enum {
    EVENT_NONE,

    // Game Events
    GAME_EVENT_SIGNAL_MAP_CHANGE,
    GAME_EVENT_MAP_CHANGE,
    GAME_EVENT_CAMERA_UPDATE_DIRECTION,
    GAME_EVENT_CAMERA_UPDATE_ROTATE,
    GAME_EVENT_CAMERA_UPDATE_TILT,
    GAME_EVENT_SWAP_WEAPONS,
    GAME_EVENT_SUMMON,
    GAME_EVENT_RESPAWN,
    GAME_EVENT_SET_POSITION,
    GAME_EVENT_DEFOG,
    GAME_EVENT_FRAMEBUFFER_SIZE_CALLBACK,
    GAME_EVENT_INTERACTABLE_CALLBACK,

    // Gui Events
    GUI_EVENT_FRAMEBUFFER_SIZE_CALLBACK,
    GUI_EVENT_CURSOR_POS_CALLBACK,
    GUI_EVENT_MOUSE_BUTTON_CALLBACK,
    GUI_EVENT_KEY_CALLBACK,
    GUI_EVENT_CONTROL_CALLBACK,
    GUI_EVENT_CHAR_CALLBACK,
    GUI_EVENT_UPDATE_WEAPON_INFO,
    GUI_EVENT_CREATE_BOSS_HEALTHBAR,
    GUI_EVENT_DESTROY_BOSS_HEALTHBAR,
    GUI_EVENT_UPDATE_BOSS_HEALTHBAR,
    GUI_EVENT_CREATE_NOTIFICATION,
    GUI_EVENT_SET_INTERACTABLE,
    GUI_EVENT_RESET_AND_CHANGE_MAP,

    // Renderer Events
    GUI_EVENT_WRITE_TEXTURE_UNITS

} EventEnum;

typedef union {
    u64 _int;
    f64 _flt;
    void* _ptr;
} Arg;

typedef struct {
    Arg arg1, arg2, arg3, arg4;
    EventEnum type;
} Event;

typedef struct {
    i32 length, capacity;
    Event* buffer;
} EventList;

typedef struct {
    EventList queues[NUM_THREADS][NUM_THREADS];
} EventContext;

static EventContext event_context;

void event_init(void)
{
    EventList* list;
    for (i32 i = 0; i < NUM_THREADS; i++) {
        for (i32 j = 0; j < NUM_THREADS; j++) {
            list = &event_context.queues[i][j];
            list->length = 0;
            list->capacity = EVENT_QUEUE_CAPACITY;
            list->buffer = st_malloc(list->capacity * sizeof(Event));
        }
    }
}

void event_cleanup(void)
{
    for (i32 i = 0; i < NUM_THREADS; i++)
        for (i32 j = 0; j < NUM_THREADS; j++)
            st_free(event_context.queues[i][j].buffer);
}

static void event_enqueue(EventList* list, Event event)
{
    if (list->length == list->capacity) {
        list->capacity += EVENT_QUEUE_CAPACITY;
        log_write(WARNING, "event queue full, resizing to %d", list->capacity);
        list->buffer = st_realloc(list->buffer, list->capacity * sizeof(Event));
    }
    list->buffer[list->length++] = event;
}

static EventList* get_event_list(const char* name)
{
    i32 thread_self_id = thread_get_self_id();
    if (thread_self_id == -1)
        log_write(FATAL, "Unrecognized thread");
    i32 thread_id = thread_get_id(name);
    if (thread_id == -1)
        log_write(FATAL, "Unrecognized thread name %s", name);
    return &event_context.queues[thread_id][thread_self_id];
}

static void execute_event(Event event)
{
    Arg arg1, arg2, arg3, arg4;
    arg1 = event.arg1;
    arg2 = event.arg2;
    arg3 = event.arg3;
    arg4 = event.arg4;
    switch (event.type) {
        case EVENT_NONE:
            break;

        // Game events
        case GAME_EVENT_SIGNAL_MAP_CHANGE:
            game_signal_change_map(arg1._int);
            break;
        case GAME_EVENT_MAP_CHANGE:
            map_create(arg1._int);
            break;
        case GAME_EVENT_CAMERA_UPDATE_DIRECTION:
            camera_update_direction(vec2_create(arg1._flt, arg2._flt));
            break;
        case GAME_EVENT_CAMERA_UPDATE_ROTATE:
            camera_update_rotation(arg1._flt);
            break;
        case GAME_EVENT_CAMERA_UPDATE_TILT:
            camera_update_tilt(arg1._flt);
            break;
        case GAME_EVENT_SWAP_WEAPONS:
            player_swap_weapons();
            break;
        case GAME_EVENT_SUMMON:
            //game_summon(arg1._int);
            break;
        case GAME_EVENT_RESPAWN:
            //player_reset();
            break;
        case GAME_EVENT_SET_POSITION:
            game_set_player_position(vec2_create(arg1._flt, arg2._flt));
            break;
        case GAME_EVENT_DEFOG:
            map_fog_clear(game_context.current_map);
            break;
        case GAME_EVENT_FRAMEBUFFER_SIZE_CALLBACK:
            game_framebuffer_size_callback();
            break;
        case GAME_EVENT_INTERACTABLE_CALLBACK:
            map_interactable_callback(arg1._ptr, arg2._ptr, arg3._ptr);
            break;

        // Gui 
        case GUI_EVENT_CURSOR_POS_CALLBACK:
            gui_cursor_pos_callback(arg1._int, arg2._int);
            break;
        case GUI_EVENT_MOUSE_BUTTON_CALLBACK:
            gui_mouse_button_callback(arg1._int, arg2._int, arg3._int);
            break;
        case GUI_EVENT_KEY_CALLBACK:
            gui_key_callback(arg1._int, arg2._int, arg3._int, arg4._int);
            break;
        case GUI_EVENT_CONTROL_CALLBACK:
            gui_control_callback(arg1._int, arg2._int);
            break;
        case GUI_EVENT_FRAMEBUFFER_SIZE_CALLBACK:
            gui_framebuffer_size_callback(arg1._int, arg2._int);
            break;
        case GUI_EVENT_CHAR_CALLBACK:
            gui_char_callback(arg1._int);
            break;
        case GUI_EVENT_UPDATE_WEAPON_INFO:
            gui_update_weapon_info(arg1._int);
            break;
        case GUI_EVENT_CREATE_BOSS_HEALTHBAR:
            //gui_create_boss_healthbar(arg1._ptr, arg2._ptr, arg3._flt, arg4._flt);
            break;
        case GUI_EVENT_UPDATE_BOSS_HEALTHBAR:
            //gui_update_boss_healthbar(arg1._ptr, arg2._flt, arg3._flt);
            break;
        case GUI_EVENT_DESTROY_BOSS_HEALTHBAR:
            //gui_destroy_boss_healthbar(arg1._ptr);
            break;
        case GUI_EVENT_CREATE_NOTIFICATION:
            gui_create_notification(arg1._ptr);
            break;
        case GUI_EVENT_SET_INTERACTABLE:
            gui_set_interactable(arg1._ptr, arg2._ptr, arg3._ptr, arg4._ptr);
            break;
        case GUI_EVENT_RESET_AND_CHANGE_MAP:
            gui_reset_and_change_map(arg1._int);
            break;

        // Renderer
        case GUI_EVENT_WRITE_TEXTURE_UNITS:
            renderer_write_texture_units();
            break;
    }
}

void event_queue_flush(void)
{
    EventList* list;
    i32 id, i, j;
    id = thread_get_self_id();
    for (i = 0; i < NUM_THREADS; i++) {
        list = &event_context.queues[id][i];
        for (j = 0; j < list->length; j++)
            execute_event(list->buffer[j]);
        list->length = 0;
        if (list->capacity > EVENT_QUEUE_CAPACITY) {
            list->capacity = EVENT_QUEUE_CAPACITY;
            list->buffer = st_realloc(list->buffer, list->capacity * sizeof(Event));
        }
    }
}

//**************************************************************************
// Game Events
//**************************************************************************

void event_create_game_signal_change_map(i32 map_id)
{
    Event event = (Event) {
        .type = GAME_EVENT_SIGNAL_MAP_CHANGE,
        .arg1._int = map_id
    };
    EventList* list = get_event_list("Game");
    event_enqueue(list, event);
}

void event_create_game_change_map(i32 map_id)
{
    Event event = (Event) {
        .type = GAME_EVENT_MAP_CHANGE,
        .arg1._int = map_id
    };
    EventList* list = get_event_list("Game");
    event_enqueue(list, event);
}

void event_create_game_camera_update_direction(vec2 mag)
{
    Event event = (Event) {
        .type = GAME_EVENT_CAMERA_UPDATE_DIRECTION,
        .arg1._flt = mag.x,
        .arg2._flt = mag.y
    };
    EventList* list = get_event_list("Game");
    event_enqueue(list, event);
}

void event_create_game_camera_update_rotation(f32 mag)
{
    Event event = (Event) {
        .type = GAME_EVENT_CAMERA_UPDATE_ROTATE,
        .arg1._flt = mag,
    };
    EventList* list = get_event_list("Game");
    event_enqueue(list, event);
}

void event_create_game_camera_update_tilt(f32 mag)
{
    Event event = (Event) {
        .type = GAME_EVENT_CAMERA_UPDATE_TILT,
        .arg1._flt = mag,
    };
    EventList* list = get_event_list("Game");
    event_enqueue(list, event);
}

void event_create_game_swap_weapons(void)
{
    Event event = (Event) {
        .type = GAME_EVENT_SWAP_WEAPONS
    };
    EventList* list = get_event_list("Game");
    event_enqueue(list, event);
}

void event_create_game_summon(i32 id)
{
    Event event = (Event) {
        .type = GAME_EVENT_SUMMON,
        .arg1._int = id
    };
    EventList* list = get_event_list("Game");
    event_enqueue(list, event);
}

void event_create_game_respawn(void)
{
    Event event = (Event) {
        .type = GAME_EVENT_RESPAWN,
    };
    EventList* list = get_event_list("Game");
    event_enqueue(list, event);
}

void event_create_game_set_player_position(vec2 position)
{
    Event event = (Event) {
        .type = GAME_EVENT_SET_POSITION,
        .arg1._flt = position.x,
        .arg2._flt = position.z
    };
    EventList* list = get_event_list("Game");
    event_enqueue(list, event);
}

void event_create_game_defog(void)
{
    Event event = (Event) {
        .type = GAME_EVENT_DEFOG
    };
    EventList* list = get_event_list("Game");
    event_enqueue(list, event);
}

void event_create_game_framebuffer_size_callback(void)
{
    Event event = (Event) {
        .type = GAME_EVENT_FRAMEBUFFER_SIZE_CALLBACK
    };
    EventList* list = get_event_list("Game");
    event_enqueue(list, event);
}

void event_create_game_interactable_callback(InteractableFuncPtr func_ptr, Map* map, MapNode* map_node)
{
    Event event = (Event) {
        .type = GAME_EVENT_INTERACTABLE_CALLBACK,
        .arg1._ptr = func_ptr,
        .arg2._ptr = map,
        .arg3._ptr = map_node,
    };
    EventList* list = get_event_list("Game");
    event_enqueue(list, event);
}

//**************************************************************************
// Gui events
//**************************************************************************

void event_create_gui_framebuffer_size_callback(u32 width, u32 height)
{
    Event event = (Event) {
        .type = GUI_EVENT_FRAMEBUFFER_SIZE_CALLBACK,
        .arg1._int = width,
        .arg2._int = height
    };
    EventList* list = get_event_list("Game");
    event_enqueue(list, event);
}

void event_create_gui_cursor_pos_callback(i32 xpos, i32 ypos)
{
    Event event = (Event) {
        .type = GUI_EVENT_CURSOR_POS_CALLBACK,
        .arg1._int = xpos,
        .arg2._int = ypos
    };
    EventList* list = get_event_list("Game");
    event_enqueue(list, event);
}

void event_create_gui_mouse_button_callback(i32 button, i32 action, i32 mods)
{
    Event event = (Event) {
        .type = GUI_EVENT_MOUSE_BUTTON_CALLBACK,
        .arg1._int = button,
        .arg2._int = action,
        .arg3._int = mods
    };
    EventList* list = get_event_list("Game");
    event_enqueue(list, event);
}

void event_create_gui_key_callback(i32 key, i32 scancode, i32 action, i32 mods)
{
    Event event = (Event) {
        .type = GUI_EVENT_KEY_CALLBACK,
        .arg1._int = key,
        .arg2._int = scancode,
        .arg3._int = action,
        .arg4._int = mods
    };
    EventList* list = get_event_list("Game");
    event_enqueue(list, event);
}

void event_create_gui_control_callback(ControlEnum ctrl, i32 action)
{
    Event event = (Event) {
        .type = GUI_EVENT_CONTROL_CALLBACK,
        .arg1._int = ctrl,
        .arg2._int = action
    };
    EventList* list = get_event_list("Game");
    event_enqueue(list, event);
}

void event_create_gui_char_callback(i32 codepoint)
{
    Event event = (Event) {
        .type = GUI_EVENT_CHAR_CALLBACK,
        .arg1._int = codepoint
    };
    EventList* list = get_event_list("Game");
    event_enqueue(list, event);
}

void event_create_gui_update_weapon_info(i32 weapon_id)
{
    Event event = (Event) {
        .type = GUI_EVENT_UPDATE_WEAPON_INFO,
        .arg1._int = weapon_id
    };
    EventList* list = get_event_list("Game");
    event_enqueue(list, event);
}

void event_create_gui_create_boss_healthbar(char* name, void* boss_ptr1, f32 health, f32 max_health)
{
    Event event = (Event) {
        .type = GUI_EVENT_CREATE_BOSS_HEALTHBAR,
        .arg1._ptr = name,
        .arg2._ptr = boss_ptr1,
        .arg3._flt = health,
        .arg4._flt = max_health,
    };
    EventList* list = get_event_list("Game");
    event_enqueue(list, event);
}

void event_create_gui_update_boss_healthbar(void* boss_ptr1, f32 health, f32 max_health)
{
    Event event = (Event) {
        .type = GUI_EVENT_UPDATE_BOSS_HEALTHBAR,
        .arg1._ptr = boss_ptr1,
        .arg2._flt = health,
        .arg3._flt = max_health
    };
    EventList* list = get_event_list("Game");
    event_enqueue(list, event);
}

void event_create_gui_destroy_boss_healthbar(void* boss_ptr1)
{
    Event event = (Event) {
        .type = GUI_EVENT_DESTROY_BOSS_HEALTHBAR,
        .arg1._ptr = boss_ptr1
    };
    EventList* list = get_event_list("Game");
    event_enqueue(list, event);
}

void event_create_gui_create_notification(char* notif)
{
    Event event = (Event) {
        .type = GUI_EVENT_CREATE_NOTIFICATION,
        .arg1._ptr = notif
    };
    EventList* list = get_event_list("Game");
    event_enqueue(list, event);
}

void event_create_gui_set_interactable(const char* desc, InteractableFuncPtr func_ptr, Map* map, MapNode* map_node)
{
    Event event = (Event) {
        .type = GUI_EVENT_SET_INTERACTABLE,
        .arg1._ptr = (void*)desc, 
        .arg2._ptr = func_ptr,
        .arg3._ptr = map,
        .arg4._ptr = map_node,
    };
    EventList* list = get_event_list("Game");
    event_enqueue(list, event);
}

void event_create_gui_reset_and_change_map(i32 map_id)
{
    Event event = (Event) {
        .type = GUI_EVENT_RESET_AND_CHANGE_MAP,
        .arg1._int = map_id
    };
    EventList* list = get_event_list("Game");
    event_enqueue(list, event);
}

//**************************************************************************
// Renderer events
//**************************************************************************

void event_create_renderer_write_texture_units(void)
{
    Event event = (Event) {
        .type = GUI_EVENT_WRITE_TEXTURE_UNITS,
    };
    EventList* list = get_event_list("Main");
    event_enqueue(list, event);
}
