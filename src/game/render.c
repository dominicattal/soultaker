/*
 * Everything in this file
 * runs on the opengl context
 * thread
*/

#include "internal.h"
#include "../renderer.h"

#define TILE_VERTEX_LENGTH           7
#define WALL_VERTEX_LENGTH           (8 * 6 * 5)
#define ENTITY_VERTEX_LENGTH_IN      8
#define ENTITY_VERTEX_LENGTH_OUT     7
#define OBSTACLE_VERTEX_LENGTH_IN    7
#define OBSTACLE_VERTEX_LENGTH_OUT   7
#define PARTICLE_VERTEX_LENGTH_IN    7
#define PARTICLE_VERTEX_LENGTH_OUT   7
#define PROJECTILE_VERTEX_LENGTH_IN  8

extern GameContext game_context;

static struct {
    GLuint vao;
    GLuint vbo;
    GLuint instance_vbo;
} tile_buffers;

static struct {
    GLuint vao;
    GLuint point_buffer;
    GLuint quad_buffer;
} proj_buffers;

static struct {
    GLuint vao;
    GLuint vbo;
} wall_buffers;

static struct {
    GLuint vao;
    GLuint vbo;
    i32 vbo_capacity;
} obstacle_buffers;

static struct {
    GLuint vao;
    GLuint vbo;
    i32 vbo_capacity;
} parstacle_buffers;

static struct {
    GLuint vao;
    GLuint vbo;
    i32 vbo_capacity;
} particle_buffers;

static struct {
    GLuint vao;
    GLuint vbo;
    i32 vbo_capacity;
} entity_buffers;

static struct {
    GLuint in;
    GLuint out;
    i32 in_capacity;
    i32 out_capacity;
} comp_buffers;

typedef struct {
    ShaderProgramEnum compute_shader;
    i32 num_objects;
    i32 object_length_in;
    i32 object_length_out;
    GLfloat* object_buffer;
    GLuint output_buffer;
    i32* output_buffer_capacity_ptr;
} ComputeShaderParams;

static void execute_compute_shader(const ComputeShaderParams* params)
{
    shader_use(params->compute_shader);
    pthread_mutex_lock(&game_context.data_mutex);
    glUniform1i(shader_get_uniform_location(params->compute_shader, "N"), params->num_objects);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, comp_buffers.in);
    if (comp_buffers.in_capacity < params->object_length_in) {
        glBufferData(GL_SHADER_STORAGE_BUFFER, params->object_length_in * sizeof(GLfloat), params->object_buffer, GL_DYNAMIC_DRAW);
        comp_buffers.in_capacity = params->object_length_in;
    }
    else
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, params->object_length_in * sizeof(GLfloat), params->object_buffer);
    pthread_mutex_unlock(&game_context.data_mutex);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, comp_buffers.out);
    if (comp_buffers.out_capacity < params->object_length_out) {
        glBufferData(GL_SHADER_STORAGE_BUFFER, params->object_length_out * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
        comp_buffers.out_capacity = params->object_length_out;
    }
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, comp_buffers.in);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, comp_buffers.out);
    glDispatchCompute((params->num_objects + 31) / 32, 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    if (*(params->output_buffer_capacity_ptr) < params->object_length_out) {
        glBindBuffer(GL_ARRAY_BUFFER, params->output_buffer);
        glBufferData(GL_ARRAY_BUFFER, params->object_length_out * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
        *(params->output_buffer_capacity_ptr) = params->object_length_out;
    }
    glBindBuffer(GL_COPY_READ_BUFFER, comp_buffers.out);
    glBindBuffer(GL_COPY_WRITE_BUFFER, params->output_buffer);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, params->object_length_out * sizeof(GLfloat));
}
    
static void update_entity_vertex_data(void)
{
    if (ENTITY_VERTEX_LENGTH_IN * game_context.entities->capacity > game_context.data_swap.entity_capacity) {
        game_context.data_swap.entity_capacity = ENTITY_VERTEX_LENGTH_IN * game_context.entities->capacity;
        size_t size = game_context.data_swap.entity_capacity * sizeof(GLfloat);
        if (game_context.data_swap.entity_buffer == NULL)
            game_context.data_swap.entity_buffer = malloc(size);
        else
            game_context.data_swap.entity_buffer = realloc(game_context.data_swap.entity_buffer, size);
        assert(game_context.entities != NULL);
    }
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

static void update_projectile_vertex_data(void)
{
    if (PROJECTILE_VERTEX_LENGTH_IN * game_context.projectiles->capacity > game_context.data_swap.proj_capacity) {
        game_context.data_swap.proj_capacity = game_context.projectiles->capacity;
        size_t size = PROJECTILE_VERTEX_LENGTH_IN * game_context.data_swap.proj_capacity * sizeof(GLfloat);
        if (game_context.data_swap.proj_buffer == NULL)
            game_context.data_swap.proj_buffer = malloc(size);
        else
            game_context.data_swap.proj_buffer = realloc(game_context.data_swap.proj_buffer, size);
        assert(game_context.projectiles != NULL);
    }
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
    if (TILE_VERTEX_LENGTH * game_context.tiles->capacity > game_context.data_swap.tile_capacity) {
        game_context.data_swap.tile_capacity = game_context.tiles->capacity;
        size_t size = TILE_VERTEX_LENGTH * game_context.data_swap.tile_capacity * sizeof(GLfloat);
        if (game_context.data_swap.tile_buffer == NULL)
            game_context.data_swap.tile_buffer = malloc(size);
        else
            game_context.data_swap.tile_buffer = realloc(game_context.data_swap.tile_buffer, size);
        assert(game_context.tiles != NULL);
    }
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
    if (WALL_VERTEX_LENGTH * game_context.walls->capacity > game_context.data_swap.wall_capacity) {
        game_context.data_swap.wall_capacity = game_context.walls->capacity;
        size_t size = WALL_VERTEX_LENGTH * game_context.data_swap.wall_capacity * sizeof(GLfloat);
        if (game_context.data_swap.wall_buffer == NULL)
            game_context.data_swap.wall_buffer = malloc(size);
        else
            game_context.data_swap.wall_buffer = realloc(game_context.data_swap.wall_buffer, size);
        assert(game_context.walls != NULL);
    }
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
    if (OBSTACLE_VERTEX_LENGTH_IN * game_context.parstacles->capacity > game_context.data_swap.parstacle_capacity) {
        game_context.data_swap.parstacle_capacity = game_context.parstacles->capacity;
        size_t size = OBSTACLE_VERTEX_LENGTH_IN * game_context.data_swap.parstacle_capacity * sizeof(GLfloat);
        if (game_context.data_swap.parstacle_buffer == NULL)
            game_context.data_swap.parstacle_buffer = malloc(size);
        else
            game_context.data_swap.parstacle_buffer = realloc(game_context.data_swap.parstacle_buffer, size);
        assert(game_context.parstacles != NULL);
    }
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
    if (OBSTACLE_VERTEX_LENGTH_IN * game_context.obstacles->capacity > game_context.data_swap.obstacle_capacity) {
        game_context.data_swap.obstacle_capacity = game_context.obstacles->capacity;
        size_t size = OBSTACLE_VERTEX_LENGTH_IN * game_context.data_swap.obstacle_capacity * sizeof(GLfloat);
        if (game_context.data_swap.obstacle_buffer == NULL)
            game_context.data_swap.obstacle_buffer = malloc(size);
        else
            game_context.data_swap.obstacle_buffer = realloc(game_context.data_swap.obstacle_buffer, size);
        assert(game_context.obstacles != NULL);
    }
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
    if (PARTICLE_VERTEX_LENGTH_IN * game_context.particles->capacity > game_context.data_swap.particle_capacity) {
        game_context.data_swap.particle_capacity = game_context.particles->capacity;
        size_t size = PARTICLE_VERTEX_LENGTH_IN * game_context.data_swap.particle_capacity * sizeof(GLfloat);
        if (game_context.data_swap.particle_buffer == NULL)
            game_context.data_swap.particle_buffer = malloc(size);
        else
            game_context.data_swap.particle_buffer = realloc(game_context.data_swap.particle_buffer, size);
        assert(game_context.particles != NULL);
    }
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

void game_update_vertex_data(void)
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

void game_render_init(void)
{
    glGenVertexArrays(1, &tile_buffers.vao);
    glGenBuffers(1, &tile_buffers.vbo);
    glGenBuffers(1, &tile_buffers.instance_vbo);
    glGenVertexArrays(1, &wall_buffers.vao);
    glGenBuffers(1, &wall_buffers.vbo);
    glGenVertexArrays(1, &entity_buffers.vao);
    glGenBuffers(1, &entity_buffers.vbo);
    entity_buffers.vbo_capacity = 0;
    glGenVertexArrays(1, &proj_buffers.vao);
    glGenBuffers(1, &proj_buffers.point_buffer);
    glGenBuffers(1, &proj_buffers.quad_buffer);
    glGenVertexArrays(1, &obstacle_buffers.vao);
    glGenBuffers(1, &obstacle_buffers.vbo);
    obstacle_buffers.vbo_capacity = 0;
    glGenVertexArrays(1, &parstacle_buffers.vao);
    glGenBuffers(1, &parstacle_buffers.vbo);
    parstacle_buffers.vbo_capacity = 0;
    glGenVertexArrays(1, &particle_buffers.vao);
    glGenBuffers(1, &particle_buffers.vbo);
    particle_buffers.vbo_capacity = 0;
    glGenBuffers(1, &comp_buffers.in);
    glGenBuffers(1, &comp_buffers.out);
    comp_buffers.in_capacity = 0;
    comp_buffers.out_capacity = 0;

    f32 quad_data[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
    };

    glBindVertexArray(tile_buffers.vao);
    glBindBuffer(GL_ARRAY_BUFFER, tile_buffers.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_data), quad_data, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, tile_buffers.instance_vbo);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);

    glBindVertexArray(wall_buffers.vao);
    glBindBuffer(GL_ARRAY_BUFFER, wall_buffers.vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(5 * sizeof(GLfloat)));
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    glBindVertexArray(entity_buffers.vao);
    glBindBuffer(GL_ARRAY_BUFFER, entity_buffers.vbo);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(4 * sizeof(GLfloat)));
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glBindVertexArray(proj_buffers.vao);
    glBindBuffer(GL_ARRAY_BUFFER, proj_buffers.quad_buffer);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(4 * sizeof(GLfloat)));
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(obstacle_buffers.vao);
    glBindBuffer(GL_ARRAY_BUFFER, obstacle_buffers.vbo);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(4 * sizeof(GLfloat)));
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glBindVertexArray(parstacle_buffers.vao);
    glBindBuffer(GL_ARRAY_BUFFER, parstacle_buffers.vbo);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(4 * sizeof(GLfloat)));
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glBindVertexArray(particle_buffers.vao);
    glBindBuffer(GL_ARRAY_BUFFER, particle_buffers.vbo);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(4 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
}

static void render_tiles(void)
{
    shader_use(SHADER_PROGRAM_TILE);
    glBindVertexArray(tile_buffers.vao);
    glBindBuffer(GL_ARRAY_BUFFER, tile_buffers.instance_vbo);
    pthread_mutex_lock(&game_context.data_mutex);
    i32 tile_length = game_context.data.tile_length;
    i32 num_tiles = tile_length / TILE_VERTEX_LENGTH;
    glBufferData(GL_ARRAY_BUFFER, tile_length * sizeof(GLfloat), game_context.data.tile_buffer, GL_STATIC_DRAW);
    pthread_mutex_unlock(&game_context.data_mutex);
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, num_tiles);
}

static void render_walls(void)
{
    shader_use(SHADER_PROGRAM_WALL);
    glBindVertexArray(wall_buffers.vao);
    glBindBuffer(GL_ARRAY_BUFFER, wall_buffers.vbo);
    pthread_mutex_lock(&game_context.data_mutex);
    i32 wall_length = game_context.data.wall_length;
    i32 num_walls = wall_length / 6;
    glBufferData(GL_ARRAY_BUFFER, wall_length * sizeof(GLfloat), game_context.data.wall_buffer, GL_STATIC_DRAW);
    pthread_mutex_unlock(&game_context.data_mutex);
    glDrawArrays(GL_TRIANGLES, 0, 6 * num_walls);
}

static void render_entities(void)
{
    i32 entity_length_in, entity_length_out, num_entities;
    entity_length_in = game_context.data.entity_length;
    num_entities = entity_length_in / ENTITY_VERTEX_LENGTH_IN;
    entity_length_out = 6 * ENTITY_VERTEX_LENGTH_OUT * num_entities;

    ComputeShaderParams params = {
        .compute_shader = SHADER_PROGRAM_ENTITY_COMP,
        .num_objects = num_entities,
        .object_length_in = entity_length_in,
        .object_length_out = entity_length_out,
        .object_buffer = game_context.data.entity_buffer,
        .output_buffer = entity_buffers.vbo,
        .output_buffer_capacity_ptr = &entity_buffers.vbo_capacity
    };

    execute_compute_shader(&params);

    shader_use(SHADER_PROGRAM_ENTITY);
    glBindVertexArray(entity_buffers.vao);
    glDrawArrays(GL_TRIANGLES, 0, 6 * num_entities);
}

static void render_obstacles(void)
{
    i32 obstacle_length_in, obstacle_length_out, num_obstacles;
    obstacle_length_in = game_context.data.obstacle_length;
    num_obstacles = obstacle_length_in / OBSTACLE_VERTEX_LENGTH_IN;
    obstacle_length_out = 6 * OBSTACLE_VERTEX_LENGTH_OUT * num_obstacles;

    ComputeShaderParams params = {
        .compute_shader = SHADER_PROGRAM_OBSTACLE_COMP,
        .num_objects = num_obstacles,
        .object_length_in = obstacle_length_in,
        .object_length_out = obstacle_length_out,
        .object_buffer = game_context.data.obstacle_buffer,
        .output_buffer = obstacle_buffers.vbo,
        .output_buffer_capacity_ptr = &obstacle_buffers.vbo_capacity
    };

    execute_compute_shader(&params);

    shader_use(SHADER_PROGRAM_OBSTACLE);
    glBindVertexArray(obstacle_buffers.vao);
    glDrawArrays(GL_TRIANGLES, 0, 6 * num_obstacles);
}

static void render_parstacles(void)
{
    i32 parstacle_length_in, parstacle_length_out, num_parstacles;
    parstacle_length_in = game_context.data.parstacle_length;
    num_parstacles = parstacle_length_in / OBSTACLE_VERTEX_LENGTH_IN;
    parstacle_length_out = 6 * OBSTACLE_VERTEX_LENGTH_OUT * num_parstacles;

    ComputeShaderParams params = {
        .compute_shader = SHADER_PROGRAM_OBSTACLE_COMP,
        .num_objects = num_parstacles,
        .object_length_in = parstacle_length_in,
        .object_length_out = parstacle_length_out,
        .object_buffer = game_context.data.parstacle_buffer,
        .output_buffer = parstacle_buffers.vbo,
        .output_buffer_capacity_ptr = &parstacle_buffers.vbo_capacity
    };

    execute_compute_shader(&params);

    shader_use(SHADER_PROGRAM_OBSTACLE);
    glBindVertexArray(parstacle_buffers.vao);
    glDrawArrays(GL_TRIANGLES, 0, 6 * num_parstacles);
}

static void render_particles(void)
{
    i32 particle_length_in, particle_length_out, num_particles;
    particle_length_in = game_context.data.particle_length;
    num_particles = particle_length_in / PARTICLE_VERTEX_LENGTH_OUT;
    particle_length_out = 6 * PARTICLE_VERTEX_LENGTH_OUT * num_particles;

    ComputeShaderParams params = {
        .compute_shader = SHADER_PROGRAM_PARTICLE_COMP,
        .num_objects = num_particles,
        .object_length_in = particle_length_in,
        .object_length_out = particle_length_out,
        .object_buffer = game_context.data.particle_buffer,
        .output_buffer = particle_buffers.vbo,
        .output_buffer_capacity_ptr = &particle_buffers.vbo_capacity
    };

    execute_compute_shader(&params);

    shader_use(SHADER_PROGRAM_PARTICLE);
    glBindVertexArray(particle_buffers.vao);
    glDrawArrays(GL_TRIANGLES, 0, 6 * num_particles);
}

static void render_parjicles(void)
{
}

static void render_projectiles(void)
{
}

void game_render(void)
{
    camera_update();
    render_tiles();
    glEnable(GL_DEPTH_TEST);
    render_walls();
    render_obstacles();
    render_parstacles();
    render_entities();
    render_projectiles();
    render_particles();
    render_parjicles();
}

void game_render_cleanup(void)
{
    glDeleteVertexArrays(1, &tile_buffers.vao);
    glDeleteVertexArrays(1, &entity_buffers.vao);
    glDeleteVertexArrays(1, &wall_buffers.vao);
    glDeleteVertexArrays(1, &proj_buffers.vao);
    glDeleteVertexArrays(1, &obstacle_buffers.vao);
    glDeleteVertexArrays(1, &particle_buffers.vao);
    glDeleteBuffers(1, &wall_buffers.vbo);
    glDeleteBuffers(1, &tile_buffers.vbo);
    glDeleteBuffers(1, &tile_buffers.instance_vbo);
    glDeleteBuffers(1, &entity_buffers.vbo);
    glDeleteBuffers(1, &proj_buffers.point_buffer);
    glDeleteBuffers(1, &proj_buffers.quad_buffer);
    glDeleteBuffers(1, &obstacle_buffers.vbo);
    glDeleteBuffers(1, &parstacle_buffers.vbo);
    glDeleteBuffers(1, &particle_buffers.vbo);
    glDeleteBuffers(1, &comp_buffers.in);
    glDeleteBuffers(1, &comp_buffers.out);
}

