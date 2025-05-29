#ifndef ENTITY_KNIGHT_H
#define ENTITY_KNIGHT_H

#include "../../src/game/internal.h"
#include <math.h>

#define NUM_STATES 3
#define INSERT(state, dir, frame, tex) \
    texture_table[state][dir][frame] = tex;

typedef struct {
    f32 state_timer;
} Data;

static GameApi* api;

__declspec(dllexport)
void entity_knight_init(GameApi* api_ext)
{
    api = api_ext;
}

__declspec(dllexport)
i32 entity_knight_texture(Entity* entity)
{
    return 0;
}

__declspec(dllexport)
void entity_knight_update(Entity* entity, f32 dt)
{
    f32 frame_length;
    switch (entity->state) {
        case 1:
            frame_length = 2 / (entity->speed + EPSILON);
            entity->frame = fmod(entity->state_timer, frame_length) > frame_length / 2;
            break;
        case 2:
            frame_length = 1 + entity->haste;
            entity->frame = fmod(entity->state_timer, frame_length) > frame_length / 2;
            break;
        default:
            break;
    }
}

__declspec(dllexport)
void entity_knight_create(Entity* entity)
{
    entity->data = malloc(sizeof(Data));
}

__declspec(dllexport)
void entity_knight_destroy(Entity* entity)
{
    free(entity->data);
}

__declspec(dllexport)
void entity_knight_cleanup(void)
{
}

#endif
