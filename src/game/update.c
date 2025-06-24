#include "internal.h"
#include <omp.h>

extern GameContext game_context;

static void collide_entity_wall(Entity* entity, Wall* wall)
{
    f32 ex, ez, er, dx, dz, wx, wz, sx, sz;
    ex = entity->position.x;
    ez = entity->position.z;
    er = entity->size / 2;
    wx = wall->position.x;
    wz = wall->position.y;
    sx = wall->size.x;
    sz = wall->size.y;
    if (!(ex + er > wx && ex - er < wx + sx
       && ez + er > wz && ez - er < wz + sz))
        return;
    ex = entity->prev_position.x;
    ez = entity->prev_position.z;
    dx = entity->position.x - entity->prev_position.x;
    dz = entity->position.z - entity->prev_position.z;
    if (ez + er > wz && ez - er < wz + sz) {
        if (dx > 0)
            entity->position.x = wx - er;
        else if (dx < 0)
            entity->position.x = wx + sx + er;
        else
            log_write(DEBUG, "dx = 0");
    } else if (ex + er > wx && ex - er < wx + sx) {
        if (dz > 0)
            entity->position.z = wz - er;
        else if (dz < 0)
            entity->position.z = wz + sz + er;
        else
            log_write(DEBUG, "dz = 0");
    }
}

static void collide_entity_obstacle(Entity* entity, Obstacle* obstacle)
{
    f32 ex, ez, er, ox, oz, or;
    vec2 dir;
    ex = entity->position.x;
    ez = entity->position.z;
    er = entity->size / 2;
    ox = obstacle->position.x;
    oz = obstacle->position.y;
    or = obstacle->size / 2;
    dir = vec2_create(ex - ox, ez - oz);
    if (vec2_mag(dir) >= er + or)
        return;
    dir = vec2_scale(vec2_normalize(dir), er + or);
    entity->position.x = ox + dir.x;
    entity->position.z = oz + dir.y;
}

static void collide_entity_projectile(Entity* entity, Projectile* projectile)
{
    bool is_entity_friendly = entity_get_flag(entity, ENTITY_FLAG_FRIENDLY);
    bool is_projectile_friendly = projectile_get_flag(projectile, PROJECTILE_FLAG_FRIENDLY);
    if (is_entity_friendly == is_projectile_friendly)
        return;

    f32 ex, ez, er, px, pz, pr;
    vec2 offset;
    ex = entity->position.x;
    ez = entity->position.z;
    er = entity->size / 2;
    px = projectile->position.x;
    pz = projectile->position.z;
    pr = projectile->size / 2;
    offset = vec2_create(ex - px, ez - pz);
    if (vec2_mag(offset) >= er + pr)
        return;

    projectile->lifetime = 0;
}

static void collide_projectile_wall(Projectile* projectile, Wall* wall)
{
    f32 px, pz, pr, wx, wz, sx, sz;
    px = projectile->position.x;
    pz = projectile->position.z;
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

static void collide_projectile_obstacle(Projectile* projectile, Obstacle* obstacle)
{
    f32 px, pz, pr, ox, oz, or;
    vec2 dir;
    px = projectile->position.x;
    pz = projectile->position.z;
    pr = projectile->size / 2;
    ox = obstacle->position.x;
    oz = obstacle->position.y;
    or = obstacle->size / 2;
    dir = vec2_create(px - ox, pz - oz);
    if (vec2_mag(dir) >= pr + or)
        return;
    projectile->lifetime = 0;
}

static void game_collide_objects(void)
{
    i32 i, j;
    for (i = 0; i < game_context.entities->length; i++) {
        Entity* entity = list_get(game_context.entities, i);
        for (j = 0; j < game_context.obstacles->length; j++) {
            Obstacle* obstacle = list_get(game_context.obstacles, j);
            collide_entity_obstacle(entity, obstacle);
        }
        for (j = 0; j < game_context.walls->length; j++) {
            Wall* wall = list_get(game_context.walls, j);
            collide_entity_wall(entity, wall);
        }
        for (j = 0; j < game_context.projectiles->length; j++) {
            Projectile* projectile = list_get(game_context.projectiles, j);
            collide_entity_projectile(entity, projectile);
        }
    }
    for (i = 0; i < game_context.projectiles->length; i++) {
        Projectile* projectile = list_get(game_context.projectiles, i);
        if (projectile->lifetime <= 0) continue;
        for (j = 0; j < game_context.obstacles->length; j++) {
            Obstacle* obstacle = list_get(game_context.obstacles, j);
            collide_projectile_obstacle(projectile, obstacle);
        }
        for (j = 0; j < game_context.walls->length; j++) {
            Wall* wall = list_get(game_context.walls, j);
            collide_projectile_wall(projectile, wall);
        }
    }
}

void game_update_objects(void)
{
    int i;
    i = 0;
    while (i < game_context.entities->length) {
        Entity* entity = list_get(game_context.entities, i);
        entity_update(entity, game_context.dt);
        if (entity->health <= 0)
            entity_destroy(list_remove(game_context.entities, i));
        else
            i++;
    }
    i = 0;
    while (i < game_context.projectiles->length) {
        Projectile* projectile = list_get(game_context.projectiles, i);
        projectile_update(projectile, game_context.dt);
        if (projectile->lifetime <= 0)
            projectile_destroy(list_remove(game_context.projectiles, i));
        else
            i++;
    }
    i = 0;
    while (i < game_context.particles->length) {
        Particle* particle = list_get(game_context.particles, i);
        particle_update(particle, game_context.dt);
        if (particle->lifetime <= 0)
            particle_destroy(list_remove(game_context.particles, i));
        else
            i++;
    }
    i = 0;
    while (i < game_context.parjicles->length) {
        Parjicle* parjicle = list_get(game_context.parjicles, i);
        parjicle_update(parjicle, game_context.dt);
        if (parjicle->lifetime <= 0)
            parjicle_destroy(list_remove(game_context.parjicles, i));
        else
            i++;
    }
    player_update(&game_context.player, game_context.dt);
}

void game_update(void)
{
    game_collide_objects();
    game_update_objects();
}

