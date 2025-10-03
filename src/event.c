#include "event.h"
#include "game/internal.h"
#include "gui/internal.h"

#define EVENT_QUEUE_CAPACITY 32

typedef enum {
    EVENT_NONE,

    // Game Events
    GAME_EVENT_MAP_LOAD,
    GAME_EVENT_CAMERA_MOVE,
    GAME_EVENT_SWAP_WEAPONS,
    GAME_EVENT_SUMMON,
    GAME_EVENT_RESPAWN,
    GAME_EVENT_SET_POSITION,
    GAME_EVENT_DEFOG,

    // Gui Events
    GUI_EVENT_FRAMEBUFFER_SIZE_CALLBACK,
    GUI_EVENT_CURSOR_POS_CALLBACK,
    GUI_EVENT_MOUSE_BUTTON_CALLBACK,
    GUI_EVENT_KEY_CALLBACK,
    GUI_EVENT_CHAR_CALLBACK,
    GUI_EVENT_UPDATE_WEAPON_INFO,
    GUI_EVENT_CREATE_BOSS_HEALTHBAR,
    GUI_EVENT_DESTROY_BOSS_HEALTHBAR,
    GUI_EVENT_UPDATE_BOSS_HEALTHBAR,

    // Renderer Events
    GUI_EVENT_WRITE_TEXTURE_UNITS

} EventEnum;

typedef union {
    u32 _int;
    f32 _flt;
} IntFloat32;

typedef struct {
    EventEnum type;
    IntFloat32 arg1, arg2, arg3, arg4;
    void* ptr;
} Event;

typedef struct {
    Event buffer[EVENT_QUEUE_CAPACITY+1];
    i32 head, tail;
} EventQueue;

typedef struct {
    EventQueue queues[NUM_THREADS][NUM_THREADS];
} EventContext;

static EventContext event_context;

static bool event_queue_full(EventQueue* queue)
{
    return queue->head == ((queue->tail + 1) % (EVENT_QUEUE_CAPACITY+1));
}

static bool event_queue_empty(EventQueue* queue)
{
    return queue->head == queue->tail;
}

static void event_enqueue(EventQueue* queue, Event event)
{
    if (event_queue_full(queue)) {
        log_write(WARNING, "Event queue at %p is full, dropping event %d", queue, event.type);
        return;
    }
    queue->buffer[queue->tail] = event;
    queue->tail = (queue->tail + 1) % (EVENT_QUEUE_CAPACITY+1);
}

static Event event_dequeue(EventQueue* queue)
{
    if (event_queue_empty(queue))
        return (Event) { .type = EVENT_NONE };
    Event event = queue->buffer[queue->head];
    queue->head = (queue->head + 1) % (EVENT_QUEUE_CAPACITY+1);
    return event;
}

static EventQueue* get_event_queue(const char* name)
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
    IntFloat32 arg1, arg2, arg3, arg4;
    void* ptr;
    arg1 = event.arg1;
    arg2 = event.arg2;
    arg3 = event.arg3;
    arg4 = event.arg4;
    ptr = event.ptr;
    switch (event.type) {
        case EVENT_NONE:
            break;

        // Game events
        case GAME_EVENT_MAP_LOAD:
            map_load(arg1._int);
            break;
        case GAME_EVENT_CAMERA_MOVE:
            camera_move(vec2_create(arg1._flt, arg2._flt));
            break;
        case GAME_EVENT_SWAP_WEAPONS:
            player_swap_weapons();
            break;
        case GAME_EVENT_SUMMON:
            game_summon(arg1._int);
            break;
        case GAME_EVENT_RESPAWN:
            player_reset();
            break;
        case GAME_EVENT_SET_POSITION:
            game_set_player_position(vec2_create(arg1._flt, arg2._flt));
            break;
        case GAME_EVENT_DEFOG:
            map_fog_clear();
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
            gui_create_boss_healthbar(ptr, arg1._flt, arg2._flt);
            break;
        case GUI_EVENT_UPDATE_BOSS_HEALTHBAR:
            gui_update_boss_healthbar(ptr, arg1._flt, arg2._flt);
            break;
        case GUI_EVENT_DESTROY_BOSS_HEALTHBAR:
            gui_destroy_boss_healthbar(ptr);
            break;

        // Renderer
        case GUI_EVENT_WRITE_TEXTURE_UNITS:
            renderer_write_texture_units();
            break;
    }
}

void event_queue_flush(void)
{
    EventQueue* queue;
    Event event;
    i32 id = thread_get_self_id();
    for (i32 i = 0; i < NUM_THREADS; i++) {
        queue = &event_context.queues[id][i];
        while ((event = event_dequeue(queue)).type != EVENT_NONE)
            execute_event(event);
    }
}

//**************************************************************************
// Game Events
//**************************************************************************

void event_create_game_map_load(i32 map_id)
{
    Event event = (Event) {
        .type = GAME_EVENT_MAP_LOAD,
        .arg1._int = map_id
    };
    EventQueue* queue = get_event_queue("Game");
    event_enqueue(queue, event);
}

void event_create_game_camera_move(vec2 mag)
{
    Event event = (Event) {
        .type = GAME_EVENT_CAMERA_MOVE,
        .arg1._flt = mag.x,
        .arg2._flt = mag.y
    };
    EventQueue* queue = get_event_queue("Game");
    event_enqueue(queue, event);
}

void event_create_game_swap_weapons(void)
{
    Event event = (Event) {
        .type = GAME_EVENT_SWAP_WEAPONS
    };
    EventQueue* queue = get_event_queue("Game");
    event_enqueue(queue, event);
}

void event_create_game_summon(i32 id)
{
    Event event = (Event) {
        .type = GAME_EVENT_SUMMON,
        .arg1._int = id
    };
    EventQueue* queue = get_event_queue("Game");
    event_enqueue(queue, event);
}

void event_create_game_respawn(void)
{
    Event event = (Event) {
        .type = GAME_EVENT_RESPAWN,
    };
    EventQueue* queue = get_event_queue("Game");
    event_enqueue(queue, event);
}

void event_create_game_set_player_position(vec2 position)
{
    Event event = (Event) {
        .type = GAME_EVENT_SET_POSITION,
        .arg1._flt = position.x,
        .arg2._flt = position.z
    };
    EventQueue* queue = get_event_queue("Game");
    event_enqueue(queue, event);
}

void event_create_game_defog(void)
{
    Event event = (Event) {
        .type = GAME_EVENT_DEFOG
    };
    EventQueue* queue = get_event_queue("Game");
    event_enqueue(queue, event);
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
    EventQueue* queue = get_event_queue("GUI");
    event_enqueue(queue, event);
}

void event_create_gui_cursor_pos_callback(i32 xpos, i32 ypos)
{
    Event event = (Event) {
        .type = GUI_EVENT_CURSOR_POS_CALLBACK,
        .arg1._int = xpos,
        .arg2._int = ypos
    };
    EventQueue* queue = get_event_queue("GUI");
    event_enqueue(queue, event);
}

void event_create_gui_mouse_button_callback(i32 button, i32 action, i32 mods)
{
    Event event = (Event) {
        .type = GUI_EVENT_MOUSE_BUTTON_CALLBACK,
        .arg1._int = button,
        .arg2._int = action,
        .arg3._int = mods
    };
    EventQueue* queue = get_event_queue("GUI");
    event_enqueue(queue, event);
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
    EventQueue* queue = get_event_queue("GUI");
    event_enqueue(queue, event);
}

void event_create_gui_char_callback(i32 codepoint)
{
    Event event = (Event) {
        .type = GUI_EVENT_CHAR_CALLBACK,
        .arg1._int = codepoint
    };
    EventQueue* queue = get_event_queue("GUI");
    event_enqueue(queue, event);
}

void event_create_gui_update_weapon_info(i32 weapon_id)
{
    Event event = (Event) {
        .type = GUI_EVENT_UPDATE_WEAPON_INFO,
        .arg1._int = weapon_id
    };
    EventQueue* queue = get_event_queue("GUI");
    event_enqueue(queue, event);
}

void event_create_gui_create_boss_healthbar(void* boss_ptr, f32 health, f32 max_health)
{
    Event event = (Event) {
        .type = GUI_EVENT_CREATE_BOSS_HEALTHBAR,
        .arg1._flt = health,
        .arg2._flt = max_health,
        .ptr = boss_ptr
    };
    EventQueue* queue = get_event_queue("GUI");
    event_enqueue(queue, event);
}

void event_create_gui_update_boss_healthbar(void* boss_ptr, f32 health, f32 max_health)
{
    Event event = (Event) {
        .type = GUI_EVENT_UPDATE_BOSS_HEALTHBAR,
        .arg1._flt = health,
        .arg2._flt = max_health,
        .ptr = boss_ptr
    };
    EventQueue* queue = get_event_queue("GUI");
    event_enqueue(queue, event);
}

void event_create_gui_destroy_boss_healthbar(void* boss_ptr)
{
    Event event = (Event) {
        .type = GUI_EVENT_DESTROY_BOSS_HEALTHBAR,
        .ptr = boss_ptr
    };
    EventQueue* queue = get_event_queue("GUI");
    event_enqueue(queue, event);
}

//**************************************************************************
// Renderer events
//**************************************************************************

void event_create_renderer_write_texture_units(void)
{
    Event event = (Event) {
        .type = GUI_EVENT_WRITE_TEXTURE_UNITS,
    };
    EventQueue* queue = get_event_queue("Main");
    event_enqueue(queue, event);
}
