#include "internal.h"
#include "../renderer.h"

GameContext game_context;

static void game_update(void)
{
    i32 i;
    i = 0;
    while (i < game_context.entities->length) {
        Entity* entity = list_get(game_context.entities, i);
        entity_update(entity, game_context.dt);
        if (entity->health <= 0)
            list_remove(game_context.entities, i);
        else
            i++;
    }
    i = 0;
    while (i < game_context.projectiles->length) {
        Projectile* proj = list_get(game_context.projectiles, i);
        projectile_update(proj, game_context.dt);
        if (proj->lifetime <= 0)
            list_remove(game_context.projectiles, i);
        else
            i++;
    }
    i = 0;
    while (i < game_context.particles->length) {
        Particle* particle = list_get(game_context.particles, i);
        particle_update(particle, game_context.dt);
        if (particle->lifetime <= 0)
            list_remove(game_context.particles, i);
        else
            i++;
    }
    i = 0;
    while (i < game_context.parjicles->length) {
        Parjicle* parjicle = list_get(game_context.parjicles, i);
        parjicle_update(parjicle, game_context.dt);
        if (parjicle->lifetime <= 0)
            list_remove(game_context.parjicles, i);
        else
            i++;
    }
    player_update(&game_context.player, game_context.dt);
}

static void collide_entity_wall(Entity* entity, Wall* wall)
{
    f32 ex, ez, er, dx, dz, wx, wz, sx, sz;
    ex = entity->position.x;
    ez = entity->position.z;
    er = entity->size / 2;
    wx = wall->position.x;
    wz = wall->position.y;
    sx = 1;
    sz = 1;
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
        else
            entity->position.x = wx + sx + er;
        entity->direction.x = 0;
    } else if (ex + er > wx && ex - er < wx + sx) {
        if (dz > 0)
            entity->position.z = wz - er;
        else
            entity->position.z = wz + sz + er;
        entity->direction.z = 0;
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

static void game_collide(void)
{
    for (i32 i = 0; i < game_context.entities->length; i++) {
        Entity* entity = list_get(game_context.entities, i);
        for (i32 j = 0; j < game_context.obstacles->length; j++) {
            Obstacle* obstacle = list_get(game_context.obstacles, j);
            collide_entity_obstacle(entity, obstacle);
        }
        for (i32 j = 0; j < game_context.walls->length; j++) {
            Wall* wall = list_get(game_context.walls, j);
            collide_entity_wall(entity, wall);
        }
    }
}

void* game_loop(void* vargp)
{
    f64 start;
    while (!game_context.kill_thread)
    {
        start = get_time();
        game_collide();
        game_update();
        game_update_vertex_data();
        game_context.dt = get_time() - start;
    }
    return NULL;
}

void game_init(void)
{
    tile_init();
    wall_init();
    entity_init();
    projectile_init();
    parstacle_init();
    obstacle_init();
    particle_init();
    parjicle_init();
    camera_init();
    game_render_init();
    game_context.data.update_tile_buffer = true;
    game_context.data.update_wall_buffer = true;
    game_context.data.update_parstacle_buffer = true;
    game_context.data.update_obstacle_buffer = true;
    game_context.data_swap.update_tile_buffer = true;
    game_context.data_swap.update_wall_buffer = true;
    game_context.data_swap.update_parstacle_buffer = true;
    game_context.data_swap.update_obstacle_buffer = true;
    pthread_mutex_init(&game_context.data_mutex, NULL);
    pthread_create(&game_context.thread_id, NULL, game_loop, NULL);
}

void game_cleanup(void)
{
    game_context.kill_thread = true;
    pthread_join(game_context.thread_id, NULL);
    pthread_mutex_destroy(&game_context.data_mutex);
    game_render_cleanup();
    camera_cleanup();
    entity_cleanup();
    projectile_cleanup();
    tile_cleanup();
    wall_cleanup();
    parstacle_cleanup();
    obstacle_cleanup();
    particle_cleanup();
    parjicle_cleanup();

    free(game_context.data.projectile_buffer);
    free(game_context.data.entity_buffer);
    free(game_context.data.tile_buffer);
    free(game_context.data.wall_buffer);
    free(game_context.data.parstacle_buffer);
    free(game_context.data.obstacle_buffer);
    free(game_context.data.particle_buffer);
    free(game_context.data.parjicle_buffer);
    free(game_context.data_swap.projectile_buffer);
    free(game_context.data_swap.entity_buffer);
    free(game_context.data_swap.tile_buffer);
    free(game_context.data_swap.wall_buffer);
    free(game_context.data_swap.parstacle_buffer);
    free(game_context.data_swap.obstacle_buffer);
    free(game_context.data_swap.particle_buffer);
    free(game_context.data_swap.parjicle_buffer);
}

f32 game_dt(void)
{
    return game_context.dt;
}
