#include "internal.h"

#define GAME_EVENT_QUEUE_CAPACITY 16
#define NUM_QUEUES 4

typedef enum {
    GAME_EVENT_NONE,
    GAME_EVENT_PRESET_LOAD,
    GAME_EVENT_CAMERA_MOVE,
    GAME_EVENT_CAMERA_ROTATE,
    GAME_EVENT_CAMERA_TILT
} GameEventEnum;

// to avoid needlessly complicated structs and
// extra heap allocation, i decided to pack
// event information into 8-byte integers
typedef struct {
    GameEventEnum type;
    u64 data1;
    u64 data2;
} GameEvent;

typedef struct {
    GameEvent buffer[GAME_EVENT_QUEUE_CAPACITY+1];
    i32 head, tail;
    pthread_t thread_id;
} GameEventQueue;

typedef struct {
    GameEventQueue queues[NUM_QUEUES];
    i32 num_queues;
} GameEventContext;

typedef struct {
    union {
        u32 _int;
        f32 _flt;
    };
} IntFloat32;

extern GameContext game_context;
static GameEventContext event_context;

static bool event_queue_full(GameEventQueue* queue)
{
    return queue->head == ((queue->tail + 1) % (GAME_EVENT_QUEUE_CAPACITY+1));
}

static bool event_queue_empty(GameEventQueue* queue)
{
    return queue->head == queue->tail;
}

static void event_enqueue(GameEventQueue* queue, GameEvent event)
{
    if (event_queue_full(queue)) {
        log_write(WARNING, "Event queue at %p is full, dropping event %d", queue, event.type);
        return;
    }
    queue->buffer[queue->tail] = event;
    queue->tail = (queue->tail + 1) % (GAME_EVENT_QUEUE_CAPACITY+1);
}

static GameEvent event_dequeue(GameEventQueue* queue)
{
    if (event_queue_empty(queue))
        return (GameEvent) { .type = GAME_EVENT_NONE };
    GameEvent event = queue->buffer[queue->head];
    queue->head = (queue->head + 1) % (GAME_EVENT_QUEUE_CAPACITY+1);
    return event;
}

static GameEventQueue* get_event_queue(void)
{
    pthread_t thread_id = pthread_self();
    for (i32 i = 0; i < event_context.num_queues; i++)
        if (pthread_equal(thread_id, event_context.queues[i].thread_id))
            return &event_context.queues[i];
    if (event_context.num_queues == NUM_QUEUES) {
        log_write(FATAL, "Out of game event queues");
        return NULL;
    }
    event_context.queues[event_context.num_queues] = (GameEventQueue) {
        .head = 0,
        .tail = 0,
        .thread_id = pthread_self()
    };
    return &event_context.queues[event_context.num_queues++];
}

static void execute_event(GameEvent event)
{
    IntFloat32 u, u1, u2;
    switch (event.type) {
        case GAME_EVENT_PRESET_LOAD:
            game_preset_load((i32)event.data1);
            break;
        case GAME_EVENT_CAMERA_MOVE:
            u2._int = event.data1 & ((1LL<<32)-1);
            u1._int = (event.data1>>32) & ((1LL<<32)-1);
            camera_move(vec2_create(u1._flt, u2._flt), game_context.dt);
            break;
        case GAME_EVENT_CAMERA_ROTATE:
            u._int = event.data1 & ((1LL<<32)-1);
            camera_rotate(u._flt, game_context.dt);
            break;
        case GAME_EVENT_CAMERA_TILT:
            u._int = event.data1 & ((1LL<<32)-1);
            camera_tilt(u._flt, game_context.dt);
            break;
        default:
            break;
    }
}

void game_event_queue_flush(void)
{
    GameEventQueue* queue;
    GameEvent event;
    for (i32 i = 0; i < event_context.num_queues; i++) {
        queue = &event_context.queues[i];
        while ((event = event_dequeue(queue)).type != GAME_EVENT_NONE)
            execute_event(event);
    }
}

void game_event_create_preset_load(i32 preset_id)
{
    GameEvent event = (GameEvent) {
        .type = GAME_EVENT_PRESET_LOAD,
        .data1 = preset_id
    };
    GameEventQueue* queue = get_event_queue();
    event_enqueue(queue, event);
}

void game_event_create_camera_move(vec2 mag)
{
    IntFloat32 u1, u2;
    u1._flt = mag.x;
    u2._flt = mag.y;
    GameEvent event = (GameEvent) {
        .type = GAME_EVENT_CAMERA_MOVE,
        .data1 = ((u64)u1._int << 32) | u2._int
    };
    GameEventQueue* queue = get_event_queue();
    event_enqueue(queue, event);
}

void game_event_create_camera_rotate(f32 mag)
{
    IntFloat32 u;
    u._flt = mag;
    GameEvent event = (GameEvent) {
        .type = GAME_EVENT_CAMERA_ROTATE,
        .data1 = u._int
    };
    GameEventQueue* queue = get_event_queue();
    event_enqueue(queue, event);
}

void game_event_create_camera_tilt(f32 mag)
{
    IntFloat32 u;
    u._flt = mag;
    GameEvent event = (GameEvent) {
        .type = GAME_EVENT_CAMERA_TILT,
        .data1 = u._int
    };
    GameEventQueue* queue = get_event_queue();
    event_enqueue(queue, event);
}
