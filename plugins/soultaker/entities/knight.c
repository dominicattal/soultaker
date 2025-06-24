#ifndef ENTITY_KNIGHT_H
#define ENTITY_KNIGHT_H

#include "../../../src/api.h"
#include <math.h>

#define NUM_STATES 3

typedef struct {
    f32 state_timer;
} Data;

static GlobalApi* api;

__declspec(dllexport)
void entity_knight_init(GlobalApi* _api)
{
    api = _api;
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
            frame_length = (1 + entity->haste) / 2;
            entity->frame = fmod(entity->state_timer, frame_length) > frame_length / 2;
            break;
        default:
            break;
    }
}

__declspec(dllexport)
void entity_knight_create(Entity* entity)
{
    entity->data = api->st_malloc(sizeof(Data));
}

__declspec(dllexport)
void entity_knight_destroy(Entity* entity)
{
    api->st_free(entity->data);
}

__declspec(dllexport)
void entity_knight_cleanup(void)
{
}

#endif
