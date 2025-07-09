#include "internal.h"
#include "../window.h"
#include "../event.h"
#include <math.h>

extern GameContext game_context;

vec3 game_get_player_position(void)
{
    if (game_context.player.entity == NULL)
        return vec3_create(0, 0, 0);
    return game_context.player.entity->position;
}

vec3 game_get_player_direction(void)
{
    if (game_context.player.entity == NULL)
        return vec3_create(0, 0, 0);
    return game_context.player.entity->direction;
}

vec2 game_get_player_facing(void)
{
    if (game_context.player.entity == NULL)
        return vec2_create(0, 0);
    return game_context.player.entity->facing;
}

void game_set_player_position(vec3 position)
{
    if (game_context.player.entity == NULL)
        return;
    game_context.player.entity->position = position;
}

void player_reset(void)
{
    if (game_context.player.entity != NULL)
        log_write(WARNING, "Did not free player entity before resetting");
    i32 knight_id = entity_get_id("knight");
    Entity* entity = entity_create(vec3_create(0, 0, 0), knight_id);
    game_context.player.entity = entity;
    game_context.player.entity->direction = vec3_create(0, 0, 0);
    game_context.player.entity->size = 1.0;
    entity_set_flag(game_context.player.entity, ENTITY_FLAG_FRIENDLY, 1);
    game_context.player.weapon.id = weapon_get_id("pointer");
    game_context.player.swap_out.id = weapon_get_id("null_pointer");
    event_create_gui_update_weapon_info(game_context.player.weapon.id);
    game_context.player.state_idle = entity_get_state_id(entity, "idle");
    game_context.player.state_walking = entity_get_state_id(entity, "walking");
    game_context.player.state_shooting = entity_get_state_id(entity, "shooting");
    //game_context.player.entity->frame_speed = 2;
}

vec3 game_get_nearest_player_position(void)
{
    if (game_context.player.entity == NULL)
        return vec3_create(0, 0, 0);
    return game_context.player.entity->position;
}

void player_update(Player* player, f32 dt)
{
    Entity* entity = player->entity;
    if (entity == NULL) return;
    player->shot_timer -= dt;
    player_shoot(player);
    if (player->shot_timer > 0) {
        if (entity->state == player->state_shooting)
            return;
        entity->state = player->state_shooting;
        entity->frame = 0;
        entity->frame_timer = 0;
        entity_set_flag(entity, ENTITY_FLAG_UPDATE_FACING, 0);
    } else {
        entity_set_flag(entity, ENTITY_FLAG_UPDATE_FACING, 1);
        if (vec3_mag(entity->direction) > 0) {
            if (entity->state == player->state_walking)
                return;
            entity->state = player->state_walking;
            entity->frame = 0;
            entity->frame_timer = 0;
        } else {
            if (entity->state == player->state_idle)
                return;
            entity->state = player->state_idle;
            entity->frame = 0;
            entity->frame_timer = 0;
        }
    }
}

void player_swap_weapons(void)
{
    i32 tmp = game_context.player.weapon.id;
    game_context.player.weapon.id = game_context.player.swap_out.id;
    game_context.player.swap_out.id = tmp;
    event_create_gui_update_weapon_info(game_context.player.weapon.id);
}

void player_shoot(Player* player)
{
    if (player->entity == NULL)
        return;
    if (!player->shooting)
        return;
    if (player->shot_timer > 0)
        return;
    player->shot_timer = 0.5;
    vec2 cursor_position = window_cursor_position();
    cursor_position.x /= window_width();
    cursor_position.y /= window_height();
    cursor_position.y = 1 - cursor_position.y;
    f32 ar = window_aspect_ratio();
    f32 zoom = camera_get_zoom();
    f32 tilt = camera_get_pitch();
    f32 rotation = camera_get_yaw();
    f32 dirx, dirz, a, b, c, r, ratio;
    vec3 direction, target;
    vec2 pos = vec2_create((cursor_position.x - 0.5) * ar, cursor_position.y - 0.5 + 1.0 / 4 / zoom);
    r = vec2_mag(pos);
    a = atan(pos.y/pos.x);
    b = PI/2 - tilt;
    c = tan(a) * tan(a) / (cos(b) * cos(b)) + 1;
    ratio = sqrt(r*r/(c * cos(a) * cos(a)));
    pos.x =  pos.x > 0 ? 1 / sqrt(c) : -1 / sqrt(c);
    pos.y = -pos.y > 0 ? sqrt(1 - 1 / c) : -sqrt(1 - 1 / c);
    dirx = pos.x * cos(rotation - HALFPI) - pos.y * sin(rotation - HALFPI);
    dirz = pos.x * sin(rotation - HALFPI) + pos.y * cos(rotation - HALFPI);
    direction = vec3_normalize(vec3_create(dirx, 0.0, dirz));
    target = vec3_sub(player->entity->position, vec3_scale(direction, -2 * zoom * r * r / ratio));
    weapon_shoot(player, direction, target);
    player->entity->facing = vec2_normalize(vec2_create(dirx, dirz));
}
