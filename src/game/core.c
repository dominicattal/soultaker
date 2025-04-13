#include "internal.h"
#include "../renderer.h"

GameContext game_context;

static void update_entity_vertex_data(void)
{
    #define FLOATS_PER_VERTEX 8
    if (FLOATS_PER_VERTEX * game_context.entities->capacity > game_context.data_swap.entity_capacity) {
        game_context.data_swap.entity_capacity = FLOATS_PER_VERTEX * game_context.entities->capacity;
        size_t size = game_context.data_swap.entity_capacity * sizeof(GLfloat);
        if (game_context.data_swap.entity_buffer == NULL)
            game_context.data_swap.entity_buffer = malloc(size);
        else
            game_context.data_swap.entity_buffer = realloc(game_context.data_swap.entity_buffer, size);
        assert(game_context.entities != NULL);
    }
    #undef FLOATS_PER_VERTEX
    game_context.data_swap.entity_length = 0;
    f32 u, v, w, h;
    i32 location;
    texture_info(TEX_KNIGHT, &u, &v, &w, &h, &location);
    #define V game_context.data_swap.entity_buffer[game_context.data_swap.entity_length++]
    for (i32 i = 0; i < game_context.entities->length; i++) {
        Entity* entity = list_get(game_context.entities, i);
        V = entity->position.x;
        V = entity->position.y;
        V = entity->position.z;
        V = u;
        V = v;
        V = w;
        V = h;
        V = location;
    }
    #undef V
}

void update_projectile_vertex_data()
{
    #define FLOATS_PER_VERTEX 8
    if (FLOATS_PER_VERTEX * game_context.projectiles->capacity > game_context.data_swap.proj_capacity) {
        game_context.data_swap.proj_capacity = game_context.projectiles->capacity;
        size_t size = FLOATS_PER_VERTEX * game_context.data_swap.proj_capacity * sizeof(GLfloat);
        if (game_context.data_swap.proj_buffer == NULL)
            game_context.data_swap.proj_buffer = malloc(size);
        else
            game_context.data_swap.proj_buffer = realloc(game_context.data_swap.proj_buffer, size);
        assert(game_context.projectiles != NULL);
    }
    #undef FLOATS_PER_VERTEX
    game_context.data_swap.proj_length = 0;
    f32 u, v, w, h;
    i32 location;
    texture_info(TEX_KNIGHT, &u, &v, &w, &h, &location);
    #define V game_context.data_swap.proj_buffer[game_context.data_swap.proj_length++]
    for (i32 i = 0; i < game_context.projectiles->length; i++) {
        Projectile* proj = list_get(game_context.projectiles, i);
        V = proj->position.x;
        V = proj->position.y;
        V = proj->position.z;
        V = u;
        V = v;
        V = w;
        V = h;
        V = location;
    }
    #undef V
}

static void update_tile_vertex_data(void)
{
    #define FLOATS_PER_VERTEX 7
    if (FLOATS_PER_VERTEX * game_context.tiles->capacity > game_context.data_swap.tile_capacity) {
        game_context.data_swap.tile_capacity = game_context.tiles->capacity;
        size_t size = FLOATS_PER_VERTEX * game_context.data_swap.tile_capacity * sizeof(GLfloat);
        if (game_context.data_swap.tile_buffer == NULL)
            game_context.data_swap.tile_buffer = malloc(size);
        else
            game_context.data_swap.tile_buffer = realloc(game_context.data_swap.tile_buffer, size);
        assert(game_context.tiles != NULL);
    }
    #undef FLOATS_PER_VERTEX
    game_context.data_swap.tile_length = 0;
    f32 u, v, w, h;
    i32 location;
    #define V game_context.data_swap.tile_buffer[game_context.data_swap.tile_length++]
    for (i32 i = 0; i < game_context.tiles->length; i++) {
        Tile* tile = list_get(game_context.tiles, i);
        texture_info(tile->tex, &u, &v, &w, &h, &location);
        V = tile->position.x;
        V = tile->position.y;
        V = u;
        V = v;
        V = w;
        V = h;
        V = location;
    }
    #undef V
}

static void update_wall_vertex_data(void)
{
    #define FLOATS_PER_VERTEX (8 * 6 * 5)
    if (FLOATS_PER_VERTEX * game_context.walls->capacity > game_context.data_swap.wall_capacity) {
        game_context.data_swap.wall_capacity = game_context.walls->capacity;
        size_t size = FLOATS_PER_VERTEX * game_context.data_swap.wall_capacity * sizeof(GLfloat);
        if (game_context.data_swap.wall_buffer == NULL)
            game_context.data_swap.wall_buffer = malloc(size);
        else
            game_context.data_swap.wall_buffer = realloc(game_context.data_swap.wall_buffer, size);
        assert(game_context.walls != NULL);
    }
    #undef FLOATS_PER_VERTEX
    game_context.data_swap.wall_length = 0;
    static f32 dx[] = {0, 0, 0, 0, 1, 1, 1, 1};
    static f32 dy[] = {0, 0, 1, 1, 0, 0, 1, 1};
    static f32 dz[] = {0, 1, 0, 1, 0, 1, 0, 1};
    static f32 tx[] = {0, 0, 1, 1};
    static f32 ty[] = {0, 1, 0, 1};
    static i32 side_order[5][4] = {
        {4, 5, 7, 6}, // +x
        {1, 3, 7, 5}, // +z
        {0, 2, 3, 1}, // -x
        {0, 4, 6, 2}, // -z
        {2, 6, 7, 3}  // +y
    };
    static i32 winding[] = {0, 1, 2, 0, 2, 3};
    f32 u, v, w, h;
    i32 location;
    i32 idx;
    #define V game_context.data_swap.wall_buffer[game_context.data_swap.wall_length++]
    for (i32 i = 0; i < game_context.walls->length; i++) {
        Wall* wall = list_get(game_context.walls, i);
        texture_info(wall->side_tex, &u, &v, &w, &h, &location);
        for (i32 side = 0; side < 4; side++) {
            for (i32 j = 0; j < 6; j++) {
                idx = side_order[side][winding[j]];
                V = wall->position.x + dx[idx]; 
                V = wall->height * dy[idx];
                V = wall->position.y + dz[idx];
                V = u + dx[winding[j]] * w;
                V = v + dy[winding[j]] * h;
                V = location;
                V = wall->position.x + 0.5;
                V = wall->position.y + 0.5;
            }
        }
        texture_info(wall->top_tex, &u, &v, &w, &h, &location);
        for (i32 j = 0; j < 6; j++) {
            idx = side_order[4][winding[j]];
            V = wall->position.x + dx[idx]; 
            V = wall->height * dy[idx];
            V = wall->position.y + dz[idx];
            V = u + tx[winding[j]] * w;
            V = v + ty[winding[j]] * h;
            V = location;
            V = wall->position.x + 0.5;
            V = wall->position.y + 0.5;
        }
    }
    #undef V
}

static void update_parstacle_vertex_data(void)
{
    #define FLOATS_PER_VERTEX 7
    if (FLOATS_PER_VERTEX * game_context.parstacles->capacity > game_context.data_swap.parstacle_capacity) {
        game_context.data_swap.parstacle_capacity = game_context.parstacles->capacity;
        size_t size = FLOATS_PER_VERTEX * game_context.data_swap.parstacle_capacity * sizeof(GLfloat);
        if (game_context.data_swap.parstacle_buffer == NULL)
            game_context.data_swap.parstacle_buffer = malloc(size);
        else
            game_context.data_swap.parstacle_buffer = realloc(game_context.data_swap.parstacle_buffer, size);
        assert(game_context.parstacles != NULL);
    }
    #undef FLOATS_PER_VERTEX
    game_context.data_swap.parstacle_length = 0;
    f32 u, v, w, h;
    i32 location;
    texture_info(TEX_BUSH, &u, &v, &w, &h, &location);
    #define V game_context.data_swap.parstacle_buffer[game_context.data_swap.parstacle_length++]
    for (i32 i = 0; i < game_context.parstacles->length; i++) {
        Parstacle* parstacle = list_get(game_context.parstacles, i);
        V = parstacle->position.x;
        V = parstacle->position.y;
        V = u;
        V = v;
        V = w;
        V = h;
        V = location;
    }
    #undef V
}

static void update_obstacle_vertex_data(void)
{
    #define FLOATS_PER_VERTEX 7
    if (FLOATS_PER_VERTEX * game_context.obstacles->capacity > game_context.data_swap.obstacle_capacity) {
        game_context.data_swap.obstacle_capacity = game_context.obstacles->capacity;
        size_t size = FLOATS_PER_VERTEX * game_context.data_swap.obstacle_capacity * sizeof(GLfloat);
        if (game_context.data_swap.obstacle_buffer == NULL)
            game_context.data_swap.obstacle_buffer = malloc(size);
        else
            game_context.data_swap.obstacle_buffer = realloc(game_context.data_swap.obstacle_buffer, size);
        assert(game_context.obstacles != NULL);
    }
    #undef FLOATS_PER_VERTEX
    game_context.data_swap.obstacle_length = 0;
    f32 u, v, w, h;
    i32 location;
    texture_info(TEX_ROCK, &u, &v, &w, &h, &location);
    #define V game_context.data_swap.obstacle_buffer[game_context.data_swap.obstacle_length++]
    for (i32 i = 0; i < game_context.obstacles->length; i++) {
        Obstacle* obstacle = list_get(game_context.obstacles, i);
        V = obstacle->position.x;
        V = obstacle->position.y;
        V = u;
        V = v;
        V = w;
        V = h;
        V = location;
    }
    #undef V
}

static void update_particle_vertex_data(void)
{
    #define FLOATS_PER_VERTEX 7
    if (FLOATS_PER_VERTEX * game_context.particles->capacity > game_context.data_swap.particle_capacity) {
        game_context.data_swap.particle_capacity = game_context.particles->capacity;
        size_t size = FLOATS_PER_VERTEX * game_context.data_swap.particle_capacity * sizeof(GLfloat);
        if (game_context.data_swap.particle_buffer == NULL)
            game_context.data_swap.particle_buffer = malloc(size);
        else
            game_context.data_swap.particle_buffer = realloc(game_context.data_swap.particle_buffer, size);
        assert(game_context.particles != NULL);
    }
    #undef FLOATS_PER_VERTEX
    game_context.data_swap.particle_length = 0;
    #define V game_context.data_swap.particle_buffer[game_context.data_swap.particle_length++]
    for (i32 i = 0; i < game_context.particles->length; i++) {
        Particle* particle = list_get(game_context.particles, i);
        V = particle->position.x;
        V = particle->position.y;
        V = particle->position.z;
        V = particle->color.x;
        V = particle->color.y;
        V = particle->color.z;
        V = particle->size;
    }
    #undef V
}

static void update_parjicle_vertex_data(void)
{
}

static void game_update_vertex_data(void)
{
    update_entity_vertex_data();
    update_projectile_vertex_data();
    update_particle_vertex_data();
    update_parjicle_vertex_data();
    if (game_context.data_swap.update_tile_buffer) {
        update_tile_vertex_data();
        game_context.data_swap.update_tile_buffer = false;
    }
    if (game_context.data_swap.update_wall_buffer) {
        update_wall_vertex_data();
        game_context.data_swap.update_wall_buffer = false;
    }
    if (game_context.data_swap.update_parstacle_buffer) {
        update_parstacle_vertex_data();
        game_context.data_swap.update_parstacle_buffer = false;
    }
    if (game_context.data_swap.update_obstacle_buffer) {
        update_obstacle_vertex_data();
        game_context.data_swap.update_obstacle_buffer = false;
    }
    pthread_mutex_lock(&game_context.data_mutex);
    GameData tmp = game_context.data;
    game_context.data = game_context.data_swap;
    game_context.data_swap = tmp;
    pthread_mutex_unlock(&game_context.data_mutex);
}

static void game_update(void)
{
    i32 i;
    static f32 kk;
    static i32 j;
    if (kk < 0) {
        entity_create(vec3_create(0, 0, 0));
        particle_create(vec3_create(0, 0, 0));
        obstacle_create(vec2_create(0, j++));
        kk = 0.5;
        game_context.data.update_obstacle_buffer = true;
        game_context.data_swap.update_obstacle_buffer = true;
    }
    kk -= game_context.dt;
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
    player_update(&game_context.player, game_context.dt);
}

static void collide_entity_wall(Entity* entity, Wall* wall)
{
    f32 ex, ez, dx, dz, wx, wz, sx, sz, r;
    ex = entity->position.x;
    ez = entity->position.z;
    dx = entity->direction.x;
    dz = entity->direction.z;
    r = entity->hitbox_radius;
    wx = wall->position.x;
    wz = wall->position.y;
    sx = 1;
    sz = 1;
    if (!(ex + r > wx && ex - r < wx + sx
       && ez + r > wz && ez - r < wz + sz))
        return;
    ex = entity->prev_position.x;
    ez = entity->prev_position.z;
    if (ez + r > wz && ez - r < wz + sz) {
        if (dx > 0)
            entity->position.x = wx - r;
        else
            entity->position.x = wx + sx + r;
        entity->direction.x = 0;
    } else if (ex + r > wx && ex - r < wx + sx) {
        if (dz > 0)
            entity->position.z = wz - r;
        else
            entity->position.z = wz + sz + r;
        entity->direction.z = 0;
    }
}

static void collide_entity_obstacle(Entity* entity, Obstacle* obstacle)
{
    f32 ex, ez, er, ox, oz, or;
    vec2 dir;
    ex = entity->position.x;
    ez = entity->position.z;
    er = entity->hitbox_radius;
    ox = obstacle->position.x;
    oz = obstacle->position.y;
    or = obstacle->hitbox_radius;
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
        for (i32 j = 0; j < game_context.walls->length; j++) {
            Wall* wall = list_get(game_context.walls, j);
            collide_entity_wall(entity, wall);
        }
        for (i32 j = 0; j < game_context.obstacles->length; j++) {
            Obstacle* obstacle = list_get(game_context.obstacles, j);
            collide_entity_obstacle(entity, obstacle);
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

    free(game_context.data_swap.proj_buffer);
    free(game_context.data.entity_buffer);
    free(game_context.data_swap.entity_buffer);
    free(game_context.data.tile_buffer);
    free(game_context.data_swap.tile_buffer);
    free(game_context.data.wall_buffer);
    free(game_context.data_swap.wall_buffer);
    free(game_context.data.proj_buffer);
    free(game_context.data.parstacle_buffer);
    free(game_context.data_swap.parstacle_buffer);
    free(game_context.data.obstacle_buffer);
    free(game_context.data_swap.obstacle_buffer);
    free(game_context.data.particle_buffer);
    free(game_context.data_swap.particle_buffer);
    free(game_context.data.parjicle_buffer);
    free(game_context.data_swap.parjicle_buffer);
}

f32 game_dt(void)
{
    return game_context.dt;
}
