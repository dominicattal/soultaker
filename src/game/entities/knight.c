#ifndef ENTITY_KNIGHT_H
#define ENTITY_KNIGHT_H

#include "../internal.h"

#define STATE_IDLE      0
#define STATE_SHOOTING  1

static TextureEnum texture_table[4][1][1];

void entity_knight_init(void)
{
    texture_table[RIGHT][STATE_IDLE][0] = TEX_KNIGHT_IDLE_RIGHT;
    texture_table[LEFT][STATE_IDLE][0] = TEX_KNIGHT_IDLE_LEFT;
    texture_table[UP][STATE_IDLE][0] = TEX_KNIGHT_IDLE_UP;
    texture_table[DOWN][STATE_IDLE][0] = TEX_KNIGHT_IDLE_DOWN;
}

TextureEnum entity_knight_get_texture(Entity* entity)
{
    i32 state = entity_get_state(entity);
    i32 dir = entity_get_direction(entity);
    return texture_table[dir][state][0];
}

#endif
