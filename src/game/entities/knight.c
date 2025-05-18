#ifndef ENTITY_KNIGHT_H
#define ENTITY_KNIGHT_H

#include "../internal.h"
#include <math.h>

typedef struct {
    f32 state_timer;
} Data;

static TextureEnum texture_table[4][NUM_KNIGHT_STATES][2];

void entity_knight_init(void)
{
    // idle
    TT_INSERT(RIGHT, KNIGHT_STATE_IDLE, 0, TEX_KNIGHT_IDLE_RIGHT);
    TT_INSERT(LEFT, KNIGHT_STATE_IDLE, 0, TEX_KNIGHT_IDLE_LEFT);
    TT_INSERT(UP, KNIGHT_STATE_IDLE, 0, TEX_KNIGHT_IDLE_UP);
    TT_INSERT(DOWN, KNIGHT_STATE_IDLE, 0, TEX_KNIGHT_IDLE_DOWN);
    // walking
    TT_INSERT(RIGHT, KNIGHT_STATE_WALKING, 0, TEX_KNIGHT_WALKING_RIGHT);
    TT_INSERT(RIGHT, KNIGHT_STATE_WALKING, 1, TEX_KNIGHT_IDLE_RIGHT);
    TT_INSERT(LEFT, KNIGHT_STATE_WALKING, 0, TEX_KNIGHT_WALKING_LEFT);
    TT_INSERT(LEFT, KNIGHT_STATE_WALKING, 1, TEX_KNIGHT_IDLE_LEFT);
    TT_INSERT(UP, KNIGHT_STATE_WALKING, 0, TEX_KNIGHT_WALKING_UP_1);
    TT_INSERT(DOWN, KNIGHT_STATE_WALKING, 0, TEX_KNIGHT_WALKING_DOWN_1);
    TT_INSERT(UP, KNIGHT_STATE_WALKING, 1, TEX_KNIGHT_WALKING_UP_2);
    TT_INSERT(DOWN, KNIGHT_STATE_WALKING, 1, TEX_KNIGHT_WALKING_DOWN_2);
    // shooting
    TT_INSERT(RIGHT, KNIGHT_STATE_SHOOTING, 0, TEX_KNIGHT_SHOOTING_RIGHT_1);
    TT_INSERT(LEFT, KNIGHT_STATE_SHOOTING, 0, TEX_KNIGHT_SHOOTING_LEFT_1);
    TT_INSERT(UP, KNIGHT_STATE_SHOOTING, 0, TEX_KNIGHT_SHOOTING_UP_1);
    TT_INSERT(DOWN, KNIGHT_STATE_SHOOTING, 0, TEX_KNIGHT_SHOOTING_DOWN_1);
    TT_INSERT(RIGHT, KNIGHT_STATE_SHOOTING, 1, TEX_KNIGHT_SHOOTING_RIGHT_2);
    TT_INSERT(LEFT, KNIGHT_STATE_SHOOTING, 1, TEX_KNIGHT_SHOOTING_LEFT_2);
    TT_INSERT(UP, KNIGHT_STATE_SHOOTING, 1, TEX_KNIGHT_SHOOTING_UP_2);
    TT_INSERT(DOWN, KNIGHT_STATE_SHOOTING, 1, TEX_KNIGHT_SHOOTING_DOWN_2);
}

TextureEnum entity_knight_get_texture(Entity* entity)
{
    i32 dir = entity_get_direction(entity);
    f32 frame_length;
    i32 frame = 0;
    switch (entity->state) {
        case KNIGHT_STATE_WALKING:
            frame_length = 2 / (entity->speed + EPSILON);
            frame = fmod(entity->state_timer, frame_length) <= frame_length / 2;
            break;
        case KNIGHT_STATE_SHOOTING:
            frame_length = 1 + entity->haste;
            frame = fmod(entity->state_timer, frame_length) <= frame_length / 2;
            break;
        default:
            break;
    }
    return texture_table[dir][entity->state][frame];
}

void entity_knight_update(Entity* entity, f32 dt)
{
}

void entity_knight_create(Entity* entity)
{
    entity->data = malloc(sizeof(Data));
}

void entity_knight_destroy(Entity* entity)
{
    free(entity->data);
}

void entity_knight_set_state(Entity* entity, i32 state)
{
    if (entity->state != state)
        entity->state_timer = 0;
    entity->state = state;
}

#endif
