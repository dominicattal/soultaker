#ifndef ENTITY_KNIGHT_H
#define ENTITY_KNIGHT_H

#include "../internal.h"

#define STATE_IDLE 0

static TextureEnum texture_table[4][1][1];

void entity_knight_init(void)
{
    texture_table[POSX][STATE_IDLE][0] = TEX_KNIGHT_IDLE_RIGHT;
    texture_table[NEGX][STATE_IDLE][0] = TEX_KNIGHT_IDLE_LEFT;
    texture_table[POSZ][STATE_IDLE][0] = TEX_KNIGHT_IDLE_UP;
    texture_table[NEGZ][STATE_IDLE][0] = TEX_KNIGHT_IDLE_DOWN;
}

TextureEnum entity_knight_get_texture(Entity* entity)
{
    i32 state = entity_get_state(entity);
    f32 entity_rad = vec2_radians(entity->facing);
    f32 camera_rad = game_context.camera.yaw;
    i32 dir = get_direction(camera_rad + entity_rad);
    return texture_table[dir][state][0];
}

#endif
