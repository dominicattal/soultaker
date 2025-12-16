#include "internal.h"
#include "../window.h"
#include "../event.h"
#include <math.h>

extern GameContext game_context;

vec2 game_get_player_position(void)
{
    if (game_context.player.entity == NULL)
        return vec2_create(0, 0);
    return game_context.player.entity->position;
}

vec2 game_get_player_direction(void)
{
    if (game_context.player.entity == NULL)
        return vec2_create(0, 0);
    return game_context.player.entity->direction;
}

vec2 game_get_player_facing(void)
{
    if (game_context.player.entity == NULL)
        return vec2_create(0, 0);
    return game_context.player.entity->facing;
}

void game_set_player_position(vec2 position)
{
    if (game_context.player.entity == NULL)
        return;
    game_context.player.entity->position = position;
}

void player_reset(void)
{
    if (game_context.player.entity != NULL) {
        log_write(WARNING, "Did not destroy player entity before resetting");
        entity_destroy(game_context.player.entity);
    }
    i32 knight_id = entity_get_id("knight");
    Entity* entity = entity_create(vec2_create(0, 0), knight_id);
    game_context.player.entity = entity;
    game_context.player.entity->direction = vec2_create(0, 0);
    game_context.player.entity->size = 1.0;
    game_context.player.entity->speed = 10;
    entity_set_flag(game_context.player.entity, ENTITY_FLAG_FRIENDLY, 1);
    game_context.player.weapon.id = weapon_get_id("pointer");
    game_context.player.swap_out.id = weapon_get_id("null_pointer");
    event_create_gui_update_weapon_info(game_context.player.weapon.id);
    game_context.player.state_idle = entity_get_state_id(entity, "idle");
    game_context.player.state_walking = entity_get_state_id(entity, "walking");
    game_context.player.state_shooting = entity_get_state_id(entity, "shooting");
    game_context.player.entity->frame_speed = 2;
}

vec2 game_get_nearest_player_position(void)
{
    if (game_context.player.entity == NULL)
        return vec2_create(0, 0);
    return game_context.player.entity->position;
}

static void update_player_state(Player* player, f32 dt)
{
    Entity* entity = player->entity;
    if (entity == NULL) return;
    player->shot_timer -= dt;
    player_shoot(player);
    if (player_is_shooting()) {
        if (entity->state == player->state_shooting)
            return;
        entity->state = player->state_shooting;
        entity->frame = 0;
        entity->frame_timer = 0;
    } else {
        if (entity_get_flag(entity, ENTITY_FLAG_MOVING)) {
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

static void update_player_stats(Player* player)
{
    Entity* entity = player->entity;
    if (entity == NULL) {
        player->stats.health = 0;
        player->stats.mana = 0;
        player->stats.souls = 0;
        return;
    }
    player->stats.health = entity->health;
    player->stats.max_health = entity->max_health;
    player->stats.mana = 50;
    player->stats.max_mana = 100;
    player->stats.souls = 50;
    player->stats.max_souls = 100;
}

void player_update(Player* player, f32 dt)
{
    if (player->entity != NULL) {
        player->position = player->entity->position;
        map_fog_explore(player->position);
    }
    update_player_state(player, dt);
    update_player_stats(player);
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
    if (player_is_shooting())
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
    f32 a, b, c, r, ratio;
    vec2 direction, target;
    const float character_offset = 1.0 / 4.0 / zoom;
    vec2 pos = vec2_create((cursor_position.x - 0.5) * ar, cursor_position.y - 0.5 + character_offset);
    // https://www.desmos.com/calculator/a7186fd475
    // basically, think of circle as screen space and ellipse inside of it as game space
    // the solution is the intersection point on the ellipse based on angle of screen space
    r = vec2_mag(pos);
    a = atan(pos.y/pos.x);
    b = PI/2 - tilt;
    c = tan(a) * tan(a) / (cos(b) * cos(b)) + 1;
    ratio = sqrt(r*r/(c * cos(a) * cos(a)));
    pos.x =  pos.x > 0 ? 1 / sqrt(c) : -1 / sqrt(c);
    pos.y = -pos.y > 0 ? sqrt(1 - 1 / c) : -sqrt(1 - 1 / c);
    direction.x = pos.x * cos(rotation - HALFPI) - pos.y * sin(rotation - HALFPI);
    direction.y = pos.x * sin(rotation - HALFPI) + pos.y * cos(rotation - HALFPI);
    direction = vec2_normalize(direction);
    target = vec2_sub(player->entity->position, vec2_scale(direction, -2 * zoom * r * r / ratio));
    weapon_shoot(player, direction, target);
    player->entity->facing = direction;
}

bool player_is_shooting(void)
{
    return game_context.player.shot_timer > 0;
}

vec2 player_position(void)
{
    return game_context.player.position;
}

f32 player_health(void)
{
    return game_context.player.stats.health;
}

f32 player_mana(void)
{
    return game_context.player.stats.mana;
}

f32 player_souls(void)
{
    return game_context.player.stats.souls;
}

f32 player_max_health(void)
{
    return game_context.player.stats.max_health;
}

f32 player_max_mana(void)
{
    return game_context.player.stats.max_mana;
}

f32 player_max_souls(void)
{
    return game_context.player.stats.max_souls;
}
