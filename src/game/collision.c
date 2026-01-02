#include "internal.h"
#include "../api.h"
#include <math.h>

extern GameContext game_context;

void collide_entity_wall(Entity* entity, Wall* wall)
{
    f32 ex, ez, er, dx, dz, wx, wz, sx, sz;
    ex = entity->position.x;
    ez = entity->position.z;
    er = entity->size / 2;
    wx = wall->position.x;
    wz = wall->position.z;
    sx = wall->size.x;
    sz = wall->size.z;
    if (!(ex + er > wx && ex - er < wx + sx
       && ez + er > wz && ez - er < wz + sz))
        return;
    ex = entity->prev_position.x;
    ez = entity->prev_position.z;
    dx = entity->position.x - entity->prev_position.x;
    dz = entity->position.z - entity->prev_position.z;
    if (ex < wx && dx > 0 && ez - er < wz + sz && ez + er > wz) {
        entity->position.x = wx - er;
        entity->direction.x = 0;
    } else if (ex > wx + sx && dx < 0 && ez - er < wz + sz && ez + er > wz) {
        entity->position.x = wx + sx + er;
        entity->direction.x = 0;
    } else if (ez < wz && dz > 0 && ex - er < wx + sx && ex + er > wx) {
        entity->position.z = wz - er;
        entity->direction.z = 0;
    } else if (ez > wz + sz && dz < 0 && ex - er < wx + sx && ex + er > wx) {
        entity->position.z = wz + sz + er;
        entity->direction.z = 0;
    }
}

void collide_entity_tile(Entity* entity, Tile* tile)
{
    if (tile->collide == NULL)
        return;

    f32 ex, ez, tx, tz, sx, sz;
    ex = entity->position.x;
    ez = entity->position.y;
    tx = tile->position.x;
    tz = tile->position.y;
    sx = sz = 1.0f;
    if (!(ex >= tx && ex < tx + sx && ez >= tz && ez < tz + sz))
        return;
    tile->collide(&game_api, entity);
}

void collide_entity_obstacle(Entity* entity, Obstacle* obstacle)
{
    f32 ex, ez, er, ox, oz, or;
    vec2 dir;
    ex = entity->position.x;
    ez = entity->position.y;
    er = entity->size / 2;
    ox = obstacle->position.x;
    oz = obstacle->position.y;
    or = obstacle->size / 2;
    dir = vec2_create(ex - ox, ez - oz);
    if (vec2_mag(dir) >= er + or)
        return;
    dir = vec2_scale(vec2_normalize(dir), er + or);
    entity->position.x = ox + dir.x;
    entity->position.y = oz + dir.y;
}

void collide_entity_projectile(Entity* entity, Projectile* projectile)
{
    if (projectile->lifetime == 0)
        return;

    bool is_entity_invulnerable = entity_get_flag(entity, ENTITY_FLAG_INVULNERABLE);
    bool is_entity_friendly = entity_get_flag(entity, ENTITY_FLAG_FRIENDLY);
    bool is_projectile_friendly = projectile_get_flag(projectile, PROJECTILE_FLAG_FRIENDLY);
    if (is_entity_friendly == is_projectile_friendly)
        return;

    f32 ex, ez, er, px, pz, pr;
    vec2 offset;
    ex = entity->position.x;
    ez = entity->position.y;
    er = entity->hitbox_radius;
    px = projectile->position.x;
    pz = projectile->position.y;
    pr = projectile->size / 2;
    offset = vec2_create(ex - px, ez - pz);
    if (vec2_mag(offset) >= er + pr)
        return;

    projectile->lifetime = 0;
    if (is_entity_invulnerable)
        return;

    entity_damage(entity, 1);
}

void collide_entity_trigger(Entity* entity, Trigger* trigger)
{
    f32 ex, ez, er, tx, tz, tr;
    vec2 offset;
    i32 i;
    ex = entity->position.x;
    ez = entity->position.y;
    er = entity->hitbox_radius;
    tx = trigger->position.x;
    tz = trigger->position.y;
    tr = trigger->radius;
    offset = vec2_create(ex - tx, ez - tz);
    if (vec2_mag(offset) >= er + tr)
        return;
    trigger_set_flag(trigger, TRIGGER_FLAG_USED, true);
    for (i = 0; i < trigger->bitset->length; i++) {
        if (entity == list_get(trigger->entities, i)) {
            bitset_set(trigger->bitset, i);
            map_handle_trigger_stay(trigger, entity);
            return;
        }
    }
    map_handle_trigger_enter(trigger, entity);
    list_append(trigger->entities, entity);
}

void collide_projectile_wall(Projectile* projectile, Wall* wall)
{
    f32 px, pz, pr, wx, wz, sx, sz;
    px = projectile->position.x;
    pz = projectile->position.y;
    pr = projectile->size / 2;
    wx = wall->position.x;
    wz = wall->position.y;
    sx = wall->size.x;
    sz = wall->size.y;
    if (!(px + pr > wx && px - pr < wx + sx
       && pz + pr > wz && pz - pr < wz + sz))
        return;
    projectile->lifetime = 0;
}

void collide_projectile_obstacle(Projectile* projectile, Obstacle* obstacle)
{
    f32 px, pz, pr, ox, oz, or;
    vec2 dir;
    px = projectile->position.x;
    pz = projectile->position.y;
    pr = projectile->size / 2;
    ox = obstacle->position.x;
    oz = obstacle->position.y;
    or = obstacle->size / 2;
    dir = vec2_create(px - ox, pz - oz);
    if (vec2_mag(dir) >= pr + or)
        return;
    projectile->lifetime = 0;
}
