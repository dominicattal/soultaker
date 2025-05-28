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

static i32 texture_table[NUM_STATES][4][2];
static GameApi* api;

__declspec(dllexport)
void entity_knight_init(GameApi* api_ext)
{
    api = api_ext;
    // idle
    INSERT(0, LEFT,  0, api->texture_get_id("knight_idle_left"));
    INSERT(0, RIGHT, 0, api->texture_get_id("knight_idle_right"));
    INSERT(0, UP,    0, api->texture_get_id("knight_idle_up"));
    INSERT(0, DOWN,  0, api->texture_get_id("knight_idle_down"));

    // walk
    INSERT(1, LEFT,  0, api->texture_get_id("knight_walking_left"));
    INSERT(1, LEFT,  1, api->texture_get_id("knight_idle_left"));
    INSERT(1, RIGHT, 0, api->texture_get_id("knight_walking_right"));
    INSERT(1, RIGHT, 1, api->texture_get_id("knight_idle_right"));
    INSERT(1, UP,    0, api->texture_get_id("knight_walking_up_1"));
    INSERT(1, UP,    1, api->texture_get_id("knight_walking_up_2"));
    INSERT(1, DOWN,  0, api->texture_get_id("knight_walking_down_1"));
    INSERT(1, DOWN,  1, api->texture_get_id("knight_walking_down_2"));

    // shoot
    INSERT(2, LEFT,  0, api->texture_get_id("knight_shooting_left_1"));
    INSERT(2, LEFT,  1, api->texture_get_id("knight_shooting_left_2"));
    INSERT(2, RIGHT, 0, api->texture_get_id("knight_shooting_right_1"));
    INSERT(2, RIGHT, 1, api->texture_get_id("knight_shooting_right_2"));
    INSERT(2, UP,    0, api->texture_get_id("knight_shooting_up_1"));
    INSERT(2, UP,    1, api->texture_get_id("knight_shooting_up_2"));
    INSERT(2, DOWN,  0, api->texture_get_id("knight_shooting_down_1"));
    INSERT(2, DOWN,  1, api->texture_get_id("knight_shooting_down_2"));
}

__declspec(dllexport)
i32 entity_knight_texture(Entity* entity)
{
    i32 dir = api->entity_get_direction(entity);
    f32 frame_length;
    i32 frame = 0;
    switch (entity->state) {
        case 1:
            frame_length = 2 / (entity->speed + EPSILON);
            frame = fmod(entity->state_timer, frame_length) > frame_length / 2;
            break;
        case 2:
            frame_length = 1 + entity->haste;
            frame = fmod(entity->state_timer, frame_length) > frame_length / 2;
            break;
        default:
            break;
    }
    return texture_table[entity->state][dir][frame];
}

__declspec(dllexport)
void entity_knight_update(Entity* entity, f32 dt)
{
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
