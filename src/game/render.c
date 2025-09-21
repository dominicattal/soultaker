#include "internal.h"
#include "../renderer.h"
#include "../window.h"

#define TILE_VERTEX_LENGTH           8
#define WALL_VERTEX_LENGTH           (8 * 6 * 5)
#define ENTITY_VERTEX_LENGTH_IN      13
#define ENTITY_VERTEX_LENGTH_OUT     7
#define OBSTACLE_VERTEX_LENGTH_IN    8
#define OBSTACLE_VERTEX_LENGTH_OUT   7
#define PARTICLE_VERTEX_LENGTH_IN    7
#define PARTICLE_VERTEX_LENGTH_OUT   7
#define PARJICLE_VERTEX_LENGTH_IN    8
#define PARJICLE_VERTEX_LENGTH_OUT   7
#define PROJECTILE_VERTEX_LENGTH_IN  11
#define PROJECTILE_VERTEX_LENGTH_OUT 7
#define SHADOW_VERTEX_LENGTH_IN      4
#define SHADOW_VERTEX_LENGTH_OUT     5
#define MAP_QUAD_VERTEX_LENGTH       4
#define MAP_CIRCLE_VERTEX_LENGTH_IN  3
#define MAP_CIRCLE_VERTEX_LENGTH_OUT 10

typedef enum {
    VAO_TILE,
    VAO_PROJECTILE,
    VAO_QUAD,
    VAO_WALL,
    VAO_OBSTACLE,
    VAO_PARSTACLE,
    VAO_PARJICLE,
    VAO_PARTICLE,
    VAO_ENTITY,
    VAO_SHADOW,
    NUM_VAOS
} GameVAOEnum;

typedef enum {
    VBO_QUAD,
    VBO_ENTITY,
    VBO_ENTITY_SHADOW,
    VBO_ENTITY_MAP,
    VBO_PROJECTILE,
    VBO_PROJECTILE_SHADOW,
    VBO_TILE,
    VBO_TILE_MAP,
    VBO_WALL,
    VBO_WALL_MAP,
    VBO_OBSTACLE,
    VBO_PARSTACLE,
    VBO_PARJICLE,
    VBO_PARTICLE,
    VBO_COMP_IN,
    VBO_COMP_OUT,
    NUM_VBOS
} GameVBOEnum;

typedef struct {
    i32 length, capacity;
    GLfloat* buffer;
    bool update;
} VertexBuffer;

typedef struct {
    VertexBuffer buffers[NUM_VBOS];
} RenderData;

typedef struct {
    RenderData* data;
    RenderData* data_swap;
    GLuint fbo, shadow_fbo, rbo;
    GLuint map_fbo, map_rbo;
    GLuint game_time_ubo;
    GLuint vaos[NUM_VAOS];
    GLuint vbos[NUM_VBOS];
    i32 vbo_capacities[NUM_VBOS];
    pthread_mutex_t mutex;
} RenderContext;

static RenderContext render_context;
extern GameContext game_context;

typedef struct {
    ShaderProgramEnum compute_shader;
    i32 num_objects;
    i32 object_length_in;
    i32 object_length_out;
    GLfloat* object_buffer;
    GLuint output_buffer;
    i32* output_buffer_capacity_ptr;
} ComputeShaderParams;

static VertexBuffer* get_vertex_buffer(GameVBOEnum type)
{
    return &render_context.data->buffers[type];
}

static VertexBuffer* get_vertex_buffer_swap(GameVBOEnum type)
{
    return &render_context.data_swap->buffers[type];
}

static void resize_vertex_buffer(VertexBuffer* vb, i32 capacity)
{
    if (vb->capacity > capacity)
        return;
    vb->capacity = capacity;
    if (vb->capacity == 0) {
        st_free(vb->buffer);
        return;
    }
    size_t size = capacity * sizeof(GLfloat);
    if (vb->buffer == NULL)
        vb->buffer = st_malloc(size);
    else
        vb->buffer = st_realloc(vb->buffer, size);

}

static void execute_compute_shader(const ComputeShaderParams* params)
{
    shader_use(params->compute_shader);
    pthread_mutex_lock(&render_context.mutex);
    glUniform1i(shader_get_uniform_location(params->compute_shader, "N"), params->num_objects);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, render_context.vbos[VBO_COMP_IN]);
    if (render_context.vbo_capacities[VBO_COMP_IN] < params->object_length_in) {
        glBufferData(GL_SHADER_STORAGE_BUFFER, params->object_length_in * sizeof(GLfloat), params->object_buffer, GL_DYNAMIC_DRAW);
        render_context.vbo_capacities[VBO_COMP_IN] = params->object_length_in;
    }
    else
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, params->object_length_in * sizeof(GLfloat), params->object_buffer);
    pthread_mutex_unlock(&render_context.mutex);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, render_context.vbos[VBO_COMP_OUT]);
    if (render_context.vbo_capacities[VBO_COMP_OUT] < params->object_length_out) {
        glBufferData(GL_SHADER_STORAGE_BUFFER, params->object_length_out * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
        render_context.vbo_capacities[VBO_COMP_OUT] = params->object_length_out;
    }
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, render_context.vbos[VBO_COMP_IN]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, render_context.vbos[VBO_COMP_OUT]);
    glDispatchCompute((params->num_objects + 31) / 32, 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    if (*(params->output_buffer_capacity_ptr) < params->object_length_out) {
        glBindBuffer(GL_ARRAY_BUFFER, params->output_buffer);
        glBufferData(GL_ARRAY_BUFFER, params->object_length_out * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
        *(params->output_buffer_capacity_ptr) = params->object_length_out;
    }
    glBindBuffer(GL_COPY_READ_BUFFER, render_context.vbos[VBO_COMP_OUT]);
    glBindBuffer(GL_COPY_WRITE_BUFFER, params->output_buffer);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, params->object_length_out * sizeof(GLfloat));
}

static void update_game_time(void)
{
    pthread_mutex_lock(&render_context.mutex);
    glBindBuffer(GL_UNIFORM_BUFFER, render_context.game_time_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GLdouble), &game_context.time);
    pthread_mutex_unlock(&render_context.mutex);
}
    
static void update_entity_vertex_data(void)
{
    VertexBuffer* vb;
    VertexBuffer* shadow_vb;
    VertexBuffer* map_vb;
    vec2 pivot, stretch;
    f32 u, v, w, h;
    i32 location;
    i32 i, j;
    Entity* entity;

    vb = get_vertex_buffer_swap(VBO_ENTITY);
    shadow_vb = get_vertex_buffer_swap(VBO_ENTITY_SHADOW);
    map_vb = get_vertex_buffer_swap(VBO_ENTITY_MAP);
    resize_vertex_buffer(vb,
            ENTITY_VERTEX_LENGTH_IN * game_context.entities->capacity);
    resize_vertex_buffer(shadow_vb,
            SHADOW_VERTEX_LENGTH_IN * game_context.entities->capacity);
    resize_vertex_buffer(map_vb,
            MAP_CIRCLE_VERTEX_LENGTH_IN * game_context.entities->capacity);

    for (i = j = 0; i < game_context.entities->length; i++) {
        entity = list_get(game_context.entities, i);
        if (map_fog_contains(entity->position))
            continue;
        texture_info(entity_get_texture(entity), &location, &u, &v, &w, &h, &pivot, &stretch);
        vb->buffer[j++] = entity->position.x;
        vb->buffer[j++] = entity->elevation;
        vb->buffer[j++] = entity->position.z;
        vb->buffer[j++] = entity->size;
        vb->buffer[j++] = u;
        vb->buffer[j++] = v;
        vb->buffer[j++] = w;
        vb->buffer[j++] = h;
        vb->buffer[j++] = location;
        vb->buffer[j++] = pivot.x;
        vb->buffer[j++] = pivot.y;
        vb->buffer[j++] = stretch.x;
        vb->buffer[j++] = stretch.y;
    }
    vb->length = j;

    for (i = j = 0; i < game_context.entities->length; i++) {
        entity = list_get(game_context.entities, i);
        if (map_fog_contains(entity->position))
            continue;
        map_vb->buffer[j++] = entity->position.x;
        map_vb->buffer[j++] = entity->position.z;
        map_vb->buffer[j++] = entity->size;
    }
    map_vb->length = j;
    
    for (i = j = 0; i < game_context.entities->length; i++) {
        entity = list_get(game_context.entities, i);
        shadow_vb->buffer[j++] = entity->position.x;
        shadow_vb->buffer[j++] = entity->elevation;
        shadow_vb->buffer[j++] = entity->position.z;
        shadow_vb->buffer[j++] = entity->size;
    }
    shadow_vb->length = j;
}

static void update_projectile_vertex_data(void)
{
    VertexBuffer* vb;
    vec2 pivot, stretch;
    f32 u, v, w, h;
    i32 location, tex;
    i32 i, j;
    Projectile* projectile;
    bool rotate_tex;

    vb = get_vertex_buffer_swap(VBO_PROJECTILE);
    resize_vertex_buffer(vb, 
            PROJECTILE_VERTEX_LENGTH_IN * game_context.projectiles->capacity);

    for (i = j = 0; i < game_context.projectiles->length; i++) {
        projectile = list_get(game_context.projectiles, i);
        if (map_fog_contains(projectile->position))
            continue;
        tex = projectile->tex;
        texture_info(tex, &location, &u, &v, &w, &h, &pivot, &stretch);
        rotate_tex = projectile_get_flag(projectile, PROJECTILE_FLAG_TEX_ROTATION);
        vb->buffer[j++] = projectile->position.x;
        vb->buffer[j++] = projectile->elevation;
        vb->buffer[j++] = projectile->position.z;
        // encode texture rotation as negative num
        vb->buffer[j++] = projectile->size * (rotate_tex ? 1 : -1);
        vb->buffer[j++] = projectile->facing;
        vb->buffer[j++] = projectile->rotation;
        vb->buffer[j++] = u;
        vb->buffer[j++] = v;
        vb->buffer[j++] = w;
        vb->buffer[j++] = h;
        vb->buffer[j++] = location; 
    }

    vb->length = j;
}

static void update_tile_vertex_data(void)
{
    VertexBuffer* vb;
    VertexBuffer* map_vb;
    vec2 pivot, stretch;
    f32 u, v, w, h;
    i32 location;
    i32 i, j;
    bool animate_horizontal_pos, animate_vertical_pos;
    bool animate_horizontal_neg, animate_vertical_neg;
    Tile* tile;

    vb = get_vertex_buffer_swap(VBO_TILE);
    map_vb = get_vertex_buffer_swap(VBO_TILE);
    resize_vertex_buffer(vb,
        TILE_VERTEX_LENGTH * game_context.tiles->capacity);
    resize_vertex_buffer(map_vb,
        MAP_QUAD_VERTEX_LENGTH * game_context.tiles->capacity);

    for (i = j = 0; i < game_context.tiles->length; i++) {
        tile = list_get(game_context.tiles, i);
        if (map_fog_contains_tile(tile))
            continue;
        animate_horizontal_pos = tile_get_flag(tile, TILE_FLAG_ANIMATE_HORIZONTAL_POS);
        animate_vertical_pos = tile_get_flag(tile, TILE_FLAG_ANIMATE_VERTICAL_POS);
        animate_horizontal_neg = tile_get_flag(tile, TILE_FLAG_ANIMATE_HORIZONTAL_NEG);
        animate_vertical_neg = tile_get_flag(tile, TILE_FLAG_ANIMATE_VERTICAL_NEG);
        texture_info(tile->tex, &location, &u, &v, &w, &h, &pivot, &stretch);
        vb->buffer[j++] = tile->position.x;
        vb->buffer[j++] = tile->position.z;
        vb->buffer[j++] = u;
        vb->buffer[j++] = v;
        vb->buffer[j++] = w;
        vb->buffer[j++] = h;
        vb->buffer[j++] = location;
        vb->buffer[j++] = animate_horizontal_pos + (animate_horizontal_neg<<1)
            + (animate_vertical_pos<<2) + (animate_vertical_neg<<3);
    }
    vb->length = j;
    return;

    for (i = j = 0; i < game_context.tiles->length; i++) {
        tile = list_get(game_context.tiles, i);
        if (map_fog_contains_tile(tile))
            continue;
        map_vb->buffer[j++] = tile->position.x;
        map_vb->buffer[j++] = tile->position.z;
        map_vb->buffer[j++] = 1.0f;
        map_vb->buffer[j++] = 1.0f;
    }
    map_vb->length = j;
}

static void update_wall_vertex_data(void)
{
    VertexBuffer* vb;
    VertexBuffer* map_vb;
    vec2 pivot, stretch;
    f32 u, v, w, h;
    i32 location;
    i32 idx;
    i32 i, j, k;
    Wall* wall;

    vb = get_vertex_buffer_swap(VBO_WALL);
    map_vb = get_vertex_buffer_swap(VBO_WALL_MAP);
    resize_vertex_buffer(vb,
            WALL_VERTEX_LENGTH * game_context.walls->capacity);
    resize_vertex_buffer(map_vb,
            MAP_QUAD_VERTEX_LENGTH * game_context.walls->capacity);

    static f32 dx[] = {0, 0, 0, 0, 1, 1, 1, 1};
    static f32 dy[] = {0, 0, 1, 1, 0, 0, 1, 1};
    static f32 dz[] = {0, 1, 0, 1, 0, 1, 0, 1};
    static f32 tx[] = {0, 1, 1, 0};
    static f32 ty[] = {0, 0, 1, 1};
    static i32 side_order[5][4] = {
        {4, 5, 7, 6}, // +x
        {3, 7, 5, 1}, // +z
        {1, 0, 2, 3}, // -x
        {0, 4, 6, 2}, // -z
        {2, 6, 7, 3}  // +y
    };
    static i32 winding[] = {0, 1, 2, 0, 2, 3};

    for (i = j = 0; i < game_context.walls->length; i++) {
        wall = list_get(game_context.walls, i);
        if (map_fog_contains_wall(wall))
            continue;
        texture_info(wall->side_tex, &location, &u, &v, &w, &h, &pivot, &stretch);
        for (i32 side = 0; side < 4; side++) {
            for (k = 0; k < 6; k++) {
                idx = side_order[side][winding[k]];
                // nvidia rounding error? epsilon fixes
                vb->buffer[j++] = wall->position.x + dx[idx] * (wall->size.x + 0.001); 
                vb->buffer[j++] = wall->height * dy[idx];
                vb->buffer[j++] = wall->position.z + dz[idx] * (wall->size.y + 0.001);
                vb->buffer[j++] = u + tx[winding[k]] * w;
                vb->buffer[j++] = v + ty[winding[k]] * h;
                vb->buffer[j++] = location;
                vb->buffer[j++] = wall->position.x + wall->size.x / 2;
                vb->buffer[j++] = wall->position.z + wall->size.y / 2;
            }
        }
        texture_info(wall->top_tex, &location, &u, &v, &w, &h, &pivot, &stretch);
        for (k = 0; k < 6; k++) {
            idx = side_order[4][winding[k]];
            vb->buffer[j++] = wall->position.x + dx[idx] * wall->size.x; 
            vb->buffer[j++] = wall->height * dy[idx];
            vb->buffer[j++] = wall->position.z + dz[idx] * wall->size.y;
            vb->buffer[j++] = u + tx[winding[k]] * w;
            vb->buffer[j++] = v + ty[winding[k]] * h;
            vb->buffer[j++] = location;
            vb->buffer[j++] = wall->position.x + wall->size.x / 2;
            vb->buffer[j++] = wall->position.z + wall->size.y / 2;
        }
    }
    vb->length = j;
    return;

    for (i = j = 0; game_context.walls->length; i++) {
        wall = list_get(game_context.walls, i);
        if (map_fog_contains_wall(wall))
            continue;
        map_vb->buffer[j++] = wall->position.x;
        map_vb->buffer[j++] = wall->position.z;
        map_vb->buffer[j++] = wall->size.x;
        map_vb->buffer[j++] = wall->size.z;
    }
    map_vb->length = j;
}

static void update_parstacle_vertex_data(void)
{
    VertexBuffer* vb;
    vec2 pivot, stretch;
    f32 u, v, w, h;
    i32 location;
    i32 i, j;
    Parstacle* parstacle;

    vb = get_vertex_buffer_swap(VBO_PARSTACLE);
    resize_vertex_buffer(vb,
            OBSTACLE_VERTEX_LENGTH_IN * game_context.parstacles->capacity);

    i32 tex = texture_get_id("bush");
    texture_info(tex, &location, &u, &v, &w, &h, &pivot, &stretch);
    
    for (i = j = 0; i < game_context.parstacles->length; i++) {
        parstacle = list_get(game_context.parstacles, i);
        if (map_fog_contains(parstacle->position))
            continue;
        vb->buffer[j++] = parstacle->position.x;
        vb->buffer[j++] = parstacle->position.z;
        vb->buffer[j++] = parstacle->size;
        vb->buffer[j++] = u;
        vb->buffer[j++] = v;
        vb->buffer[j++] = w;
        vb->buffer[j++] = h;
        vb->buffer[j++] = location;
    }

    vb->length = j;
}

static void update_obstacle_vertex_data(void)
{
    VertexBuffer* vb;
    Obstacle* obstacle;
    vec2 pivot, stretch;
    f32 u, v, w, h;
    i32 location;
    i32 i, j;

    vb = get_vertex_buffer_swap(VBO_OBSTACLE);
    resize_vertex_buffer(vb,
            OBSTACLE_VERTEX_LENGTH_IN * game_context.obstacles->capacity);

    i32 tex = texture_get_id("rock");
    texture_info(tex, &location, &u, &v, &w, &h, &pivot, &stretch);
    
    for (i = j = 0; i < game_context.obstacles->length; i++) {
        obstacle = list_get(game_context.obstacles, i);
        if (map_fog_contains(obstacle->position))
            continue;
        vb->buffer[j++] = obstacle->position.x;
        vb->buffer[j++] = obstacle->position.z;
        vb->buffer[j++] = obstacle->size;
        vb->buffer[j++] = u;
        vb->buffer[j++] = v;
        vb->buffer[j++] = w;
        vb->buffer[j++] = h;
        vb->buffer[j++] = location;
    }

    vb->length = j;
}

static void update_particle_vertex_data(void)
{
    VertexBuffer* vb;
    Particle* particle;
    i32 i, j;

    vb = get_vertex_buffer_swap(VBO_PARTICLE);
    resize_vertex_buffer(vb,
            PARTICLE_VERTEX_LENGTH_IN * game_context.particles->capacity);
   
    for (i = j = 0; i < game_context.particles->length; i++) {
        particle = list_get(game_context.particles, i);
        if (map_fog_contains(vec2_create(particle->position.x, particle->position.z)))
            continue;
        vb->buffer[j++] = particle->position.x;
        vb->buffer[j++] = particle->position.z;
        vb->buffer[j++] = particle->position.z;
        vb->buffer[j++] = particle->color.x;
        vb->buffer[j++] = particle->color.y;
        vb->buffer[j++] = particle->color.z;
        vb->buffer[j++] = particle->size;
    }
    
    vb->length = j;
}

static void update_parjicle_vertex_data(void)
{
    VertexBuffer* vb;
    Parjicle* parjicle;
    bool rotate_tex;
    i32 i, j;

    vb = get_vertex_buffer_swap(VBO_PARJICLE);
    resize_vertex_buffer(vb,
            PARJICLE_VERTEX_LENGTH_IN * game_context.parjicles->capacity);
   
    for (i = j = 0; i < game_context.parjicles->length; i++) {
        parjicle = list_get(game_context.parjicles, i);
        if (map_fog_contains(vec2_create(parjicle->position.x, parjicle->position.z)))
            continue;
        rotate_tex = parjicle_is_flag_set(parjicle, PARJICLE_FLAG_TEX_ROTATION);
        vb->buffer[j++] = parjicle->position.x;
        vb->buffer[j++] = parjicle->position.z;
        vb->buffer[j++] = parjicle->position.z;
        vb->buffer[j++] = parjicle->color.x;
        vb->buffer[j++] = parjicle->color.y;
        vb->buffer[j++] = parjicle->color.z;
        vb->buffer[j++] = parjicle->size * (rotate_tex ? 1 : -1);
        vb->buffer[j++] = parjicle->rotation;
    }
    
    vb->length = j;
}

static void update_vertex_buffer_data(VertexBuffer* vb, void (*func)(void))
{
    if (vb->update) {
        func();
        vb->update = false;
    }
}

void game_update_vertex_data(void)
{
    VertexBuffer* vb;
    vb = get_vertex_buffer_swap(VBO_ENTITY);
    vb->update = true;
    vb = get_vertex_buffer_swap(VBO_PROJECTILE);
    vb->update = true;
    vb = get_vertex_buffer_swap(VBO_PARTICLE);
    vb->update = true;
    vb = get_vertex_buffer_swap(VBO_PARJICLE);
    vb->update = true;

    update_vertex_buffer_data(
            get_vertex_buffer_swap(VBO_ENTITY),
            update_entity_vertex_data);
    update_vertex_buffer_data(
            get_vertex_buffer_swap(VBO_PROJECTILE),
            update_projectile_vertex_data);
    update_vertex_buffer_data(
            get_vertex_buffer_swap(VBO_PARTICLE),
            update_particle_vertex_data);
    update_vertex_buffer_data(
            get_vertex_buffer_swap(VBO_PARJICLE),
            update_parjicle_vertex_data);
    update_vertex_buffer_data(
            get_vertex_buffer_swap(VBO_TILE),
            update_tile_vertex_data);
    update_vertex_buffer_data(
            get_vertex_buffer_swap(VBO_WALL),
            update_wall_vertex_data);
    update_vertex_buffer_data(
            get_vertex_buffer_swap(VBO_PARSTACLE),
            update_parstacle_vertex_data);
    update_vertex_buffer_data(
            get_vertex_buffer_swap(VBO_OBSTACLE),
            update_obstacle_vertex_data);

    pthread_mutex_lock(&render_context.mutex);
    RenderData* tmp = render_context.data;
    render_context.data = render_context.data_swap;
    render_context.data_swap = tmp;
    pthread_mutex_unlock(&render_context.mutex);
}

static void render_tiles(void)
{
    VertexBuffer* vb;
    vb = get_vertex_buffer(VBO_TILE);

    shader_use(SHADER_PROGRAM_TILE);
    glBindVertexArray(render_context.vaos[VAO_TILE]);
    glBindBuffer(GL_ARRAY_BUFFER, render_context.vbos[VAO_TILE]);
    pthread_mutex_lock(&render_context.mutex);
    i32 tile_length = vb->length;
    i32 num_tiles = tile_length / TILE_VERTEX_LENGTH;
    glBufferData(GL_ARRAY_BUFFER, tile_length * sizeof(GLfloat), vb->buffer, GL_STATIC_DRAW);
    pthread_mutex_unlock(&render_context.mutex);
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, num_tiles);
}

static void render_walls(void)
{
    VertexBuffer* vb;
    vb = get_vertex_buffer(VBO_WALL);

    shader_use(SHADER_PROGRAM_WALL);
    glBindVertexArray(render_context.vaos[VAO_WALL]);
    glBindBuffer(GL_ARRAY_BUFFER, render_context.vbos[VAO_WALL]);
    pthread_mutex_lock(&render_context.mutex);
    i32 wall_length = vb->length;
    i32 num_walls = wall_length / 6;
    glBufferData(GL_ARRAY_BUFFER, wall_length * sizeof(GLfloat), vb->buffer, GL_STATIC_DRAW);
    pthread_mutex_unlock(&render_context.mutex);
    glDrawArrays(GL_TRIANGLES, 0, 6 * num_walls);
}

static void render_entities(void)
{
    VertexBuffer* vb;
    vb = get_vertex_buffer(VBO_ENTITY);

    i32 entity_length_in, entity_length_out, num_entities;
    entity_length_in = vb->length;
    num_entities = entity_length_in / ENTITY_VERTEX_LENGTH_IN;
    entity_length_out = 6 * ENTITY_VERTEX_LENGTH_OUT * num_entities;

    ComputeShaderParams params = {
        .compute_shader = SHADER_PROGRAM_ENTITY_COMP,
        .num_objects = num_entities,
        .object_length_in = entity_length_in,
        .object_length_out = entity_length_out,
        .object_buffer = vb->buffer,
        .output_buffer = render_context.vbos[VAO_ENTITY],
        .output_buffer_capacity_ptr = &render_context.vbo_capacities[VAO_ENTITY]
    };

    execute_compute_shader(&params);

    shader_use(SHADER_PROGRAM_ENTITY);
    glBindVertexArray(render_context.vaos[VAO_ENTITY]);
    glDrawArrays(GL_TRIANGLES, 0, 6 * num_entities);
}

static void render_obstacles(void)
{
    VertexBuffer* vb;
    vb = get_vertex_buffer(VBO_OBSTACLE);

    i32 obstacle_length_in, obstacle_length_out, num_obstacles;
    obstacle_length_in = vb->length;
    num_obstacles = obstacle_length_in / OBSTACLE_VERTEX_LENGTH_IN;
    obstacle_length_out = 6 * OBSTACLE_VERTEX_LENGTH_OUT * num_obstacles;

    ComputeShaderParams params = {
        .compute_shader = SHADER_PROGRAM_OBSTACLE_COMP,
        .num_objects = num_obstacles,
        .object_length_in = obstacle_length_in,
        .object_length_out = obstacle_length_out,
        .object_buffer = vb->buffer,
        .output_buffer = render_context.vbos[VAO_OBSTACLE],
        .output_buffer_capacity_ptr = &render_context.vbo_capacities[VAO_OBSTACLE]
    };

    execute_compute_shader(&params);

    shader_use(SHADER_PROGRAM_OBSTACLE);
    glBindVertexArray(render_context.vaos[VAO_OBSTACLE]);
    glDrawArrays(GL_TRIANGLES, 0, 6 * num_obstacles);
}

static void render_parstacles(void)
{
    VertexBuffer* vb;
    vb = get_vertex_buffer(VBO_PARSTACLE);

    i32 parstacle_length_in, parstacle_length_out, num_parstacles;
    parstacle_length_in = vb->length;
    num_parstacles = parstacle_length_in / OBSTACLE_VERTEX_LENGTH_IN;
    parstacle_length_out = 6 * OBSTACLE_VERTEX_LENGTH_OUT * num_parstacles;

    ComputeShaderParams params = {
        .compute_shader = SHADER_PROGRAM_OBSTACLE_COMP,
        .num_objects = num_parstacles,
        .object_length_in = parstacle_length_in,
        .object_length_out = parstacle_length_out,
        .object_buffer = vb->buffer,
        .output_buffer = render_context.vbos[VAO_PARSTACLE],
        .output_buffer_capacity_ptr = &render_context.vbo_capacities[VAO_PARSTACLE]
    };

    execute_compute_shader(&params);

    shader_use(SHADER_PROGRAM_OBSTACLE);
    glBindVertexArray(render_context.vaos[VAO_PARSTACLE]);
    glDrawArrays(GL_TRIANGLES, 0, 6 * num_parstacles);
}

static void render_particles(void)
{
    VertexBuffer* vb;
    vb = get_vertex_buffer(VBO_PARTICLE);

    i32 particle_length_in, particle_length_out, num_particles;
    particle_length_in = vb->length;
    num_particles = particle_length_in / PARTICLE_VERTEX_LENGTH_IN;
    particle_length_out = 6 * PARTICLE_VERTEX_LENGTH_OUT * num_particles;

    ComputeShaderParams params = {
        .compute_shader = SHADER_PROGRAM_PARTICLE_COMP,
        .num_objects = num_particles,
        .object_length_in = particle_length_in,
        .object_length_out = particle_length_out,
        .object_buffer = vb->buffer,
        .output_buffer = render_context.vbos[VAO_PARTICLE],
        .output_buffer_capacity_ptr = &render_context.vbo_capacities[VAO_PARTICLE]
    };

    execute_compute_shader(&params);

    shader_use(SHADER_PROGRAM_PARTICLE);
    glBindVertexArray(render_context.vaos[VAO_PARTICLE]);
    glDrawArrays(GL_TRIANGLES, 0, 6 * num_particles);
}

static void render_parjicles(void)
{
    VertexBuffer* vb;
    vb = get_vertex_buffer(VBO_PARJICLE);

    i32 parjicle_length_in, parjicle_length_out, num_parjicles;
    parjicle_length_in = vb->length;
    num_parjicles = parjicle_length_in / PARJICLE_VERTEX_LENGTH_IN;
    parjicle_length_out = 6 * PARJICLE_VERTEX_LENGTH_OUT * num_parjicles;

    ComputeShaderParams params = {
        .compute_shader = SHADER_PROGRAM_PARJICLE_COMP,
        .num_objects = num_parjicles,
        .object_length_in = parjicle_length_in,
        .object_length_out = parjicle_length_out,
        .object_buffer = vb->buffer,
        .output_buffer = render_context.vbos[VAO_PARJICLE],
        .output_buffer_capacity_ptr = &render_context.vbo_capacities[VAO_PARJICLE]
    };

    execute_compute_shader(&params);

    shader_use(SHADER_PROGRAM_PARJICLE);
    glBindVertexArray(render_context.vaos[VAO_PARJICLE]);
    glDrawArrays(GL_TRIANGLES, 0, 6 * num_parjicles);
}

static void render_projectiles(void)
{
    VertexBuffer* vb;
    vb = get_vertex_buffer(VBO_PROJECTILE);

    i32 projectile_length_in, projectile_length_out, num_projectiles;
    projectile_length_in = vb->length;
    num_projectiles = projectile_length_in / PROJECTILE_VERTEX_LENGTH_IN;
    projectile_length_out = 6 * PROJECTILE_VERTEX_LENGTH_OUT * num_projectiles;

    ComputeShaderParams params = {
        .compute_shader = SHADER_PROGRAM_PROJECTILE_COMP,
        .num_objects = num_projectiles,
        .object_length_in = projectile_length_in,
        .object_length_out = projectile_length_out,
        .object_buffer = vb->buffer,
        .output_buffer = render_context.vbos[VAO_PROJECTILE],
        .output_buffer_capacity_ptr = &render_context.vbo_capacities[VAO_PROJECTILE]
    };

    execute_compute_shader(&params);

    shader_use(SHADER_PROGRAM_PROJECTILE);
    glBindVertexArray(render_context.vaos[VAO_PROJECTILE]);
    glDrawArrays(GL_TRIANGLES, 0, 6 * num_projectiles);
}

static void render_shadows(void)
{
    return;
    VertexBuffer* vb;
    ComputeShaderParams params;

    i32 shadow_length_in, shadow_length_out, num_shadows;
    vb = get_vertex_buffer(VBO_ENTITY_SHADOW);
    shadow_length_in = vb->length;
    num_shadows = shadow_length_in / SHADOW_VERTEX_LENGTH_IN;
    shadow_length_out = 6 * SHADOW_VERTEX_LENGTH_OUT * num_shadows;

    params = (ComputeShaderParams) {
        .compute_shader = SHADER_PROGRAM_SHADOW_COMP,
        .num_objects = num_shadows,
        .object_length_in = shadow_length_in,
        .object_length_out = shadow_length_out,
        .object_buffer = vb->buffer,
        .output_buffer = render_context.vbos[VAO_SHADOW],
        .output_buffer_capacity_ptr = &render_context.vbo_capacities[VAO_SHADOW]
    };

    execute_compute_shader(&params);

    shader_use(SHADER_PROGRAM_SHADOW);
    glBindVertexArray(render_context.vaos[VAO_SHADOW]);
    glDrawArrays(GL_TRIANGLES, 0, 6 * num_shadows);
}

void game_render_init(void)
{
    pthread_mutex_init(&render_context.mutex, NULL);
    render_context.data = st_calloc(1, sizeof(RenderData));
    render_context.data_swap = st_calloc(1, sizeof(RenderData));

    glGenBuffers(1, &render_context.game_time_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, render_context.game_time_ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GLdouble), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, UBO_INDEX_GAME_TIME, render_context.game_time_ubo);

    glGenVertexArrays(NUM_VAOS, &render_context.vaos[0]);
    glGenBuffers(NUM_VBOS, &render_context.vbos[0]);
    glGenBuffers(1, &render_context.vbos[VBO_COMP_IN]);
    glGenBuffers(1, &render_context.vbos[VBO_COMP_OUT]);

    f32 quad_data[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
    };

    glBindVertexArray(render_context.vaos[VAO_QUAD]);
    glBindBuffer(GL_ARRAY_BUFFER, render_context.vbos[VAO_QUAD]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_data), quad_data, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(render_context.vaos[VAO_TILE]);
    glBindBuffer(GL_ARRAY_BUFFER, render_context.vbos[VAO_QUAD]);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, render_context.vbos[VAO_TILE]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(7 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);

    glBindVertexArray(render_context.vaos[VAO_WALL]);
    glBindBuffer(GL_ARRAY_BUFFER, render_context.vbos[VAO_WALL]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(5 * sizeof(GLfloat)));
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    glBindVertexArray(render_context.vaos[VAO_ENTITY]);
    glBindBuffer(GL_ARRAY_BUFFER, render_context.vbos[VAO_ENTITY]);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(4 * sizeof(GLfloat)));
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glBindVertexArray(render_context.vaos[VAO_PROJECTILE]);
    glBindBuffer(GL_ARRAY_BUFFER, render_context.vbos[VAO_PROJECTILE]);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(4 * sizeof(GLfloat)));
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(render_context.vaos[VAO_OBSTACLE]);
    glBindBuffer(GL_ARRAY_BUFFER, render_context.vbos[VAO_OBSTACLE]);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(4 * sizeof(GLfloat)));
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glBindVertexArray(render_context.vaos[VAO_PARSTACLE]);
    glBindBuffer(GL_ARRAY_BUFFER, render_context.vbos[VAO_PARSTACLE]);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(4 * sizeof(GLfloat)));
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glBindVertexArray(render_context.vaos[VAO_PARTICLE]);
    glBindBuffer(GL_ARRAY_BUFFER, render_context.vbos[VAO_PARTICLE]);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(4 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(render_context.vaos[VAO_PARJICLE]);
    glBindBuffer(GL_ARRAY_BUFFER, render_context.vbos[VAO_PARJICLE]);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(4 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(render_context.vaos[VAO_SHADOW]);
    glBindBuffer(GL_ARRAY_BUFFER, render_context.vbos[VAO_SHADOW]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    GLuint unit, name;

    glGenRenderbuffers(1, &render_context.rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, render_context.rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, window_width(), window_height());

    unit = texture_get_unit(TEX_GAME_SCENE);
    name = texture_get_name(TEX_GAME_SCENE);
    glGenFramebuffers(1, &render_context.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, render_context.fbo);
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, name);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_width(), window_height(), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, name, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, render_context.rbo);
    
    unit = texture_get_unit(TEX_GAME_SHADOW_SCENE);
    name = texture_get_name(TEX_GAME_SHADOW_SCENE);
    glGenFramebuffers(1, &render_context.shadow_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, render_context.shadow_fbo);
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, name);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_width(), window_height(), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, name, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, render_context.rbo);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void game_render(void)
{
    GLuint loc, unit;

    if (game_context.halt_render)
        return;

    update_game_time();
    camera_update();

    glEnable(GL_STENCIL_TEST);
    glEnable(GL_DEPTH_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    glBindFramebuffer(GL_FRAMEBUFFER, render_context.fbo);
    renderer_check_framebuffer_status(GL_FRAMEBUFFER, "game");
    GLenum buffer[] = { GL_COLOR_ATTACHMENT0 };
    const f32 color[4] = {0.0f, 1.0f, 1.0f, 1.0f};
    //glStencilMask(0x01); // stencil mask affects glClear
    glDrawBuffers(1, buffer);
    glClearBufferfv(GL_COLOR, 0, color);
    glClearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);

    glStencilFunc(GL_ALWAYS, 1, 0x01);
    glStencilMask(0x01);
    render_walls();

    glStencilFunc(GL_NOTEQUAL, 1, 0x01);
    glStencilMask(0x00);
    render_tiles();

    glStencilFunc(GL_ALWAYS, 1, 0x01);
    glStencilMask(0x01);
    render_obstacles();
    render_parstacles();
    render_entities();
    render_projectiles();
    render_particles();
    render_parjicles();

    glBindFramebuffer(GL_FRAMEBUFFER, render_context.shadow_fbo);
    renderer_check_framebuffer_status(GL_FRAMEBUFFER, "shadow");
    glClearBufferfv(GL_COLOR, 0, color);
    glClearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);
    glDisable(GL_DEPTH_TEST);
    glStencilMask(0x00);
    shader_use(SHADER_PROGRAM_SCREEN);
    unit = texture_get_unit(TEX_GAME_SCENE);
    loc = shader_get_uniform_location(SHADER_PROGRAM_SCREEN, "screenTex"); 
    glUniform1i(loc, unit);
    glBindVertexArray(render_context.vaos[VAO_QUAD]);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glStencilFunc(GL_NOTEQUAL, 1, 0x01);
    render_shadows();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_STENCIL_TEST);
    shader_use(SHADER_PROGRAM_SCREEN);
    unit = texture_get_unit(TEX_GAME_SHADOW_SCENE);
    loc = shader_get_uniform_location(SHADER_PROGRAM_SCREEN, "screenTex"); 
    glUniform1i(loc, unit);
    glBindVertexArray(render_context.vaos[VAO_QUAD]);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void game_render_framebuffer_size_callback(void)
{
    if (render_context.fbo == 0 
     || render_context.rbo == 0
     || render_context.shadow_fbo == 0)
        return;

    i32 width = window_width();
    i32 height = window_height();
    if (width == 0 || height == 0)
        return;

    GLuint name;
    name = texture_get_name(TEX_GAME_SCENE);
    glBindFramebuffer(GL_FRAMEBUFFER, render_context.fbo);
    glBindTexture(GL_TEXTURE_2D, name);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height,
            0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    name = texture_get_name(TEX_GAME_SHADOW_SCENE);
    glBindFramebuffer(GL_FRAMEBUFFER, render_context.shadow_fbo);
    glBindTexture(GL_TEXTURE_2D, name);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height,
            0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glBindRenderbuffer(GL_RENDERBUFFER, render_context.rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void game_render_update_obstacles(void)
{
    VertexBuffer* vb;
    vb = get_vertex_buffer(VBO_WALL);
    vb->update = true;
    vb = get_vertex_buffer_swap(VBO_WALL);
    vb->update = true;
}

void game_render_update_parstacles(void)
{
    VertexBuffer* vb;
    vb = get_vertex_buffer(VBO_PARSTACLE);
    vb->update = true;
    vb = get_vertex_buffer_swap(VBO_PARSTACLE);
    vb->update = true;
}

void game_render_update_tiles(void)
{
    VertexBuffer* vb;
    vb = get_vertex_buffer(VBO_TILE);
    vb->update = true;
    vb = get_vertex_buffer_swap(VBO_TILE);
    vb->update = true;
}

void game_render_update_walls(void)
{
    VertexBuffer* vb;
    vb = get_vertex_buffer(VBO_WALL);
    vb->update = true;
    vb = get_vertex_buffer_swap(VBO_WALL);
    vb->update = true;
}

void game_render_cleanup(void)
{
    pthread_mutex_destroy(&render_context.mutex);
    glDeleteVertexArrays(NUM_VAOS, &render_context.vaos[0]);
    glDeleteBuffers(NUM_VBOS, &render_context.vbos[0]);
    glDeleteBuffers(1, &render_context.game_time_ubo);
    glDeleteFramebuffers(1, &render_context.fbo);
    glDeleteFramebuffers(1, &render_context.shadow_fbo);
    glDeleteRenderbuffers(1, &render_context.rbo);

    for (i32 i = 0; i < NUM_VBOS; i++) {
        st_free(render_context.data->buffers[i].buffer);
        st_free(render_context.data_swap->buffers[i].buffer);
    }
    st_free(render_context.data);
    st_free(render_context.data_swap);
}

