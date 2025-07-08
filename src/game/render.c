#include "internal.h"
#include "../renderer.h"
#include "../window.h"

#define TILE_VERTEX_LENGTH           8
#define WALL_VERTEX_LENGTH           (8 * 6 * 5)
#define ENTITY_VERTEX_LENGTH_IN      11
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

typedef struct {
    i32 length, capacity;
    GLfloat* buffer;
    bool update;
} VertexBuffer;

typedef struct {
    VertexBuffer tile;
    VertexBuffer wall;
    VertexBuffer parstacle;
    VertexBuffer obstacle;
    VertexBuffer entity;
    VertexBuffer projectile;
    VertexBuffer particle;
    VertexBuffer parjicle;
    VertexBuffer shadow;
    bool update_tile_buffer;
    bool update_wall_buffer;
    bool update_parstacle_buffer;
    bool update_obstacle_buffer;
} RenderData;

typedef struct {
    RenderData* data;
    RenderData* data_swap;
    pthread_mutex_t mutex;
    GLuint fbo, rbo, tex;
} RenderContext;

static RenderContext render_context;
extern GameContext game_context;

static struct {
    GLuint vao;
    GLuint vbo;
} tile_buffers;

static struct {
    GLuint vao;
    GLuint vbo;
    i32 vbo_capacity;
} projectile_buffers;

static struct {
    GLuint vao;
    GLuint vbo;
} quad_buffers;

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
} parjicle_buffers;

static struct {
    GLuint vao;
    GLuint vbo;
    i32 vbo_capacity;
} entity_buffers;

static struct {
    GLuint vao;
    GLuint vbo;
    GLuint fbo;
    i32 vbo_capacity;
} shadow_buffers;

static struct {
    GLuint in;
    GLuint out;
    i32 in_capacity;
    i32 out_capacity;
} comp_buffers;

static u32 game_time_ubo;

typedef struct {
    ShaderProgramEnum compute_shader;
    i32 num_objects;
    i32 object_length_in;
    i32 object_length_out;
    GLfloat* object_buffer;
    GLuint output_buffer;
    i32* output_buffer_capacity_ptr;
} ComputeShaderParams;

static void resize_vertex_buffer(VertexBuffer* vb, i32 capacity)
{
    if (vb->capacity > capacity)
        return;
    vb->capacity = capacity;
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
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, comp_buffers.in);
    if (comp_buffers.in_capacity < params->object_length_in) {
        glBufferData(GL_SHADER_STORAGE_BUFFER, params->object_length_in * sizeof(GLfloat), params->object_buffer, GL_DYNAMIC_DRAW);
        comp_buffers.in_capacity = params->object_length_in;
    }
    else
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, params->object_length_in * sizeof(GLfloat), params->object_buffer);
    pthread_mutex_unlock(&render_context.mutex);

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

static void update_game_time(void)
{
    pthread_mutex_lock(&render_context.mutex);
    glBindBuffer(GL_UNIFORM_BUFFER, game_time_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GLdouble), &game_context.time);
    pthread_mutex_unlock(&render_context.mutex);
}
    
static void update_entity_vertex_data(void)
{
    VertexBuffer* vb;
    VertexBuffer* shadow_vb;
    vec2 pivot, stretch;
    f32 u, v, w, h;
    i32 location;
    i32 i, j, k;
    Entity* entity;

    vb = &render_context.data_swap->entity;
    shadow_vb = &render_context.data_swap->shadow;
    resize_vertex_buffer(vb,
            ENTITY_VERTEX_LENGTH_IN * game_context.entities->capacity);
    resize_vertex_buffer(shadow_vb,
            SHADOW_VERTEX_LENGTH_IN * game_context.entities->capacity);

    k = 0;
    for (i = j = 0; i < game_context.entities->length; i++) {
        entity = list_get(game_context.entities, i);
        texture_info(entity_get_texture(entity), &location, &u, &v, &w, &h, &pivot, &stretch);
        vb->buffer[j++] = entity->position.x;
        vb->buffer[j++] = entity->position.y;
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

        shadow_vb->buffer[k++] = entity->position.x;
        shadow_vb->buffer[k++] = entity->position.y;
        shadow_vb->buffer[k++] = entity->position.z;
        shadow_vb->buffer[k++] = entity->size;
    }

    render_context.data_swap->entity.length = j;
    render_context.data_swap->shadow.length = k;
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

    vb = &render_context.data_swap->projectile;
    resize_vertex_buffer(vb, 
            PROJECTILE_VERTEX_LENGTH_IN * game_context.projectiles->capacity);

    for (i = j = 0; i < game_context.projectiles->length; i++) {
        projectile = list_get(game_context.projectiles, i);
        tex = projectile->tex;
        texture_info(tex, &location, &u, &v, &w, &h, &pivot, &stretch);
        rotate_tex = projectile_get_flag(projectile, PROJECTILE_FLAG_TEX_ROTATION);
        vb->buffer[j++] = projectile->position.x;
        vb->buffer[j++] = projectile->position.y;
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

    render_context.data_swap->projectile.length = j;
}

static void update_tile_vertex_data(void)
{
    VertexBuffer* vb;
    vec2 pivot, stretch;
    f32 u, v, w, h;
    i32 location;
    i32 i, j;
    bool animate_horizontal_pos, animate_vertical_pos;
    bool animate_horizontal_neg, animate_vertical_neg;
    Tile* tile;

    vb = &render_context.data_swap->tile;
    resize_vertex_buffer(vb,
        TILE_VERTEX_LENGTH * game_context.tiles->capacity);

    for (i = j = 0; i < game_context.tiles->length; i++) {
        tile = list_get(game_context.tiles, i);
        animate_horizontal_pos = tile_get_flag(tile, TILE_FLAG_ANIMATE_HORIZONTAL_POS);
        animate_vertical_pos = tile_get_flag(tile, TILE_FLAG_ANIMATE_VERTICAL_POS);
        animate_horizontal_neg = tile_get_flag(tile, TILE_FLAG_ANIMATE_HORIZONTAL_NEG);
        animate_vertical_neg = tile_get_flag(tile, TILE_FLAG_ANIMATE_VERTICAL_NEG);
        texture_info(tile->tex, &location, &u, &v, &w, &h, &pivot, &stretch);
        vb->buffer[j++] = tile->position.x;
        vb->buffer[j++] = tile->position.y;
        vb->buffer[j++] = u;
        vb->buffer[j++] = v;
        vb->buffer[j++] = w;
        vb->buffer[j++] = h;
        vb->buffer[j++] = location;
        vb->buffer[j++] = animate_horizontal_pos + (animate_horizontal_neg<<1)
            + (animate_vertical_pos<<2) + (animate_vertical_neg<<3);
    }

    render_context.data_swap->tile.length = j;
}

static void update_wall_vertex_data(void)
{
    VertexBuffer* vb;
    vec2 pivot, stretch;
    f32 u, v, w, h;
    i32 location;
    i32 idx;
    i32 i, j, k;
    Wall* wall;

    vb = &render_context.data_swap->wall;
    resize_vertex_buffer(vb,
            WALL_VERTEX_LENGTH * game_context.walls->capacity);

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
        texture_info(wall->side_tex, &location, &u, &v, &w, &h, &pivot, &stretch);
        for (i32 side = 0; side < 4; side++) {
            for (k = 0; k < 6; k++) {
                idx = side_order[side][winding[k]];
                vb->buffer[j++] = wall->position.x + dx[idx] * wall->size.x; 
                vb->buffer[j++] = wall->height * dy[idx];
                vb->buffer[j++] = wall->position.y + dz[idx] * wall->size.y;
                vb->buffer[j++] = u + tx[winding[k]] * w;
                vb->buffer[j++] = v + ty[winding[k]] * h;
                vb->buffer[j++] = location;
                vb->buffer[j++] = wall->position.x + wall->size.x / 2;
                vb->buffer[j++] = wall->position.y + wall->size.y / 2;
            }
        }
        texture_info(wall->top_tex, &location, &u, &v, &w, &h, &pivot, &stretch);
        for (k = 0; k < 6; k++) {
            idx = side_order[4][winding[k]];
            vb->buffer[j++] = wall->position.x + dx[idx] * wall->size.x; 
            vb->buffer[j++] = wall->height * dy[idx];
            vb->buffer[j++] = wall->position.y + dz[idx] * wall->size.y;
            vb->buffer[j++] = u + tx[winding[k]] * w;
            vb->buffer[j++] = v + ty[winding[k]] * h;
            vb->buffer[j++] = location;
            vb->buffer[j++] = wall->position.x + wall->size.x / 2;
            vb->buffer[j++] = wall->position.y + wall->size.y / 2;
        }
    }

    render_context.data_swap->wall.length = j;
}

static void update_parstacle_vertex_data(void)
{
    VertexBuffer* vb;
    vec2 pivot, stretch;
    f32 u, v, w, h;
    i32 location;
    i32 i, j;
    Parstacle* parstacle;

    vb = &render_context.data_swap->parstacle;
    resize_vertex_buffer(vb,
            OBSTACLE_VERTEX_LENGTH_IN * game_context.parstacles->capacity);

    i32 tex = texture_get_id("bush");
    texture_info(tex, &location, &u, &v, &w, &h, &pivot, &stretch);
    
    for (i = j = 0; i < game_context.parstacles->length; i++) {
        parstacle = list_get(game_context.parstacles, i);
        vb->buffer[j++] = parstacle->position.x;
        vb->buffer[j++] = parstacle->position.y;
        vb->buffer[j++] = parstacle->size;
        vb->buffer[j++] = u;
        vb->buffer[j++] = v;
        vb->buffer[j++] = w;
        vb->buffer[j++] = h;
        vb->buffer[j++] = location;
    }

    render_context.data_swap->parstacle.length = j;
}

static void update_obstacle_vertex_data(void)
{
    VertexBuffer* vb;
    Obstacle* obstacle;
    vec2 pivot, stretch;
    f32 u, v, w, h;
    i32 location;
    i32 i, j;

    vb = &render_context.data_swap->obstacle;
    resize_vertex_buffer(vb,
            OBSTACLE_VERTEX_LENGTH_IN * game_context.obstacles->capacity);

    i32 tex = texture_get_id("rock");
    texture_info(tex, &location, &u, &v, &w, &h, &pivot, &stretch);
    
    for (i = j = 0; i < game_context.obstacles->length; i++) {
        obstacle = list_get(game_context.obstacles, i);
        vb->buffer[j++] = obstacle->position.x;
        vb->buffer[j++] = obstacle->position.y;
        vb->buffer[j++] = obstacle->size;
        vb->buffer[j++] = u;
        vb->buffer[j++] = v;
        vb->buffer[j++] = w;
        vb->buffer[j++] = h;
        vb->buffer[j++] = location;
    }

    render_context.data_swap->obstacle.length = j;
}

static void update_particle_vertex_data(void)
{
    VertexBuffer* vb;
    Particle* particle;
    i32 i, j;

    vb = &render_context.data_swap->particle;
    resize_vertex_buffer(vb,
            PARTICLE_VERTEX_LENGTH_IN * game_context.particles->capacity);
   
    for (i = j = 0; i < game_context.particles->length; i++) {
        particle = list_get(game_context.particles, i);
        vb->buffer[j++] = particle->position.x;
        vb->buffer[j++] = particle->position.y;
        vb->buffer[j++] = particle->position.z;
        vb->buffer[j++] = particle->color.x;
        vb->buffer[j++] = particle->color.y;
        vb->buffer[j++] = particle->color.z;
        vb->buffer[j++] = particle->size;
    }
    
    render_context.data_swap->particle.length = j;
}

static void update_parjicle_vertex_data(void)
{
    VertexBuffer* vb;
    Parjicle* parjicle;
    bool rotate_tex;
    i32 i, j;

    vb = &render_context.data_swap->parjicle;
    resize_vertex_buffer(vb,
            PARJICLE_VERTEX_LENGTH_IN * game_context.parjicles->capacity);
   
    for (i = j = 0; i < game_context.parjicles->length; i++) {
        parjicle = list_get(game_context.parjicles, i);
        rotate_tex = parjicle_is_flag_set(parjicle, PARJICLE_FLAG_TEX_ROTATION);
        vb->buffer[j++] = parjicle->position.x;
        vb->buffer[j++] = parjicle->position.y;
        vb->buffer[j++] = parjicle->position.z;
        vb->buffer[j++] = parjicle->color.x;
        vb->buffer[j++] = parjicle->color.y;
        vb->buffer[j++] = parjicle->color.z;
        vb->buffer[j++] = parjicle->size * (rotate_tex ? 1 : -1);
        vb->buffer[j++] = parjicle->rotation;
    }
    
    render_context.data_swap->parjicle.length = j;
}

void game_update_vertex_data(void)
{
    update_entity_vertex_data();
    update_projectile_vertex_data();
    update_particle_vertex_data();
    update_parjicle_vertex_data();
    if (render_context.data_swap->update_tile_buffer) {
        update_tile_vertex_data();
        render_context.data_swap->update_tile_buffer = false;
    }
    if (render_context.data_swap->update_wall_buffer) {
        update_wall_vertex_data();
        render_context.data_swap->update_wall_buffer = false;
    }
    if (render_context.data_swap->update_parstacle_buffer) {
        update_parstacle_vertex_data();
        render_context.data_swap->update_parstacle_buffer = false;
    }
    if (render_context.data_swap->update_obstacle_buffer) {
        update_obstacle_vertex_data();
        render_context.data_swap->update_obstacle_buffer = false;
    }
    pthread_mutex_lock(&render_context.mutex);
    RenderData* tmp = render_context.data;
    render_context.data = render_context.data_swap;
    render_context.data_swap = tmp;
    pthread_mutex_unlock(&render_context.mutex);
}

static void render_tiles(void)
{
    shader_use(SHADER_PROGRAM_TILE);
    glBindVertexArray(tile_buffers.vao);
    glBindBuffer(GL_ARRAY_BUFFER, tile_buffers.vbo);
    pthread_mutex_lock(&render_context.mutex);
    i32 tile_length = render_context.data->tile.length;
    i32 num_tiles = tile_length / TILE_VERTEX_LENGTH;
    glBufferData(GL_ARRAY_BUFFER, tile_length * sizeof(GLfloat), render_context.data->tile.buffer, GL_STATIC_DRAW);
    pthread_mutex_unlock(&render_context.mutex);
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, num_tiles);
}

static void render_walls(void)
{
    shader_use(SHADER_PROGRAM_WALL);
    glBindVertexArray(wall_buffers.vao);
    glBindBuffer(GL_ARRAY_BUFFER, wall_buffers.vbo);
    pthread_mutex_lock(&render_context.mutex);
    i32 wall_length = render_context.data->wall.length;
    i32 num_walls = wall_length / 6;
    glBufferData(GL_ARRAY_BUFFER, wall_length * sizeof(GLfloat), render_context.data->wall.buffer, GL_STATIC_DRAW);
    pthread_mutex_unlock(&render_context.mutex);
    glDrawArrays(GL_TRIANGLES, 0, 6 * num_walls);
}

static void render_entities(void)
{
    i32 entity_length_in, entity_length_out, num_entities;
    entity_length_in = render_context.data->entity.length;
    num_entities = entity_length_in / ENTITY_VERTEX_LENGTH_IN;
    entity_length_out = 6 * ENTITY_VERTEX_LENGTH_OUT * num_entities;

    ComputeShaderParams params = {
        .compute_shader = SHADER_PROGRAM_ENTITY_COMP,
        .num_objects = num_entities,
        .object_length_in = entity_length_in,
        .object_length_out = entity_length_out,
        .object_buffer = render_context.data->entity.buffer,
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
    obstacle_length_in = render_context.data->obstacle.length;
    num_obstacles = obstacle_length_in / OBSTACLE_VERTEX_LENGTH_IN;
    obstacle_length_out = 6 * OBSTACLE_VERTEX_LENGTH_OUT * num_obstacles;

    ComputeShaderParams params = {
        .compute_shader = SHADER_PROGRAM_OBSTACLE_COMP,
        .num_objects = num_obstacles,
        .object_length_in = obstacle_length_in,
        .object_length_out = obstacle_length_out,
        .object_buffer = render_context.data->obstacle.buffer,
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
    parstacle_length_in = render_context.data->parstacle.length;
    num_parstacles = parstacle_length_in / OBSTACLE_VERTEX_LENGTH_IN;
    parstacle_length_out = 6 * OBSTACLE_VERTEX_LENGTH_OUT * num_parstacles;

    ComputeShaderParams params = {
        .compute_shader = SHADER_PROGRAM_OBSTACLE_COMP,
        .num_objects = num_parstacles,
        .object_length_in = parstacle_length_in,
        .object_length_out = parstacle_length_out,
        .object_buffer = render_context.data->parstacle.buffer,
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
    particle_length_in = render_context.data->particle.length;
    num_particles = particle_length_in / PARTICLE_VERTEX_LENGTH_IN;
    particle_length_out = 6 * PARTICLE_VERTEX_LENGTH_OUT * num_particles;

    ComputeShaderParams params = {
        .compute_shader = SHADER_PROGRAM_PARTICLE_COMP,
        .num_objects = num_particles,
        .object_length_in = particle_length_in,
        .object_length_out = particle_length_out,
        .object_buffer = render_context.data->particle.buffer,
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
    i32 parjicle_length_in, parjicle_length_out, num_parjicles;
    parjicle_length_in = render_context.data->parjicle.length;
    num_parjicles = parjicle_length_in / PARJICLE_VERTEX_LENGTH_IN;
    parjicle_length_out = 6 * PARJICLE_VERTEX_LENGTH_OUT * num_parjicles;

    ComputeShaderParams params = {
        .compute_shader = SHADER_PROGRAM_PARJICLE_COMP,
        .num_objects = num_parjicles,
        .object_length_in = parjicle_length_in,
        .object_length_out = parjicle_length_out,
        .object_buffer = render_context.data->parjicle.buffer,
        .output_buffer = parjicle_buffers.vbo,
        .output_buffer_capacity_ptr = &parjicle_buffers.vbo_capacity
    };

    execute_compute_shader(&params);

    shader_use(SHADER_PROGRAM_PARJICLE);
    glBindVertexArray(parjicle_buffers.vao);
    glDrawArrays(GL_TRIANGLES, 0, 6 * num_parjicles);
}

static void render_projectiles(void)
{
    i32 projectile_length_in, projectile_length_out, num_projectiles;
    projectile_length_in = render_context.data->projectile.length;
    num_projectiles = projectile_length_in / PROJECTILE_VERTEX_LENGTH_IN;
    projectile_length_out = 6 * PROJECTILE_VERTEX_LENGTH_OUT * num_projectiles;

    ComputeShaderParams params = {
        .compute_shader = SHADER_PROGRAM_PROJECTILE_COMP,
        .num_objects = num_projectiles,
        .object_length_in = projectile_length_in,
        .object_length_out = projectile_length_out,
        .object_buffer = render_context.data->projectile.buffer,
        .output_buffer = projectile_buffers.vbo,
        .output_buffer_capacity_ptr = &projectile_buffers.vbo_capacity
    };

    execute_compute_shader(&params);

    shader_use(SHADER_PROGRAM_PROJECTILE);
    glBindVertexArray(projectile_buffers.vao);
    glDrawArrays(GL_TRIANGLES, 0, 6 * num_projectiles);
}

static void render_shadows(void)
{
    ComputeShaderParams params;

    i32 shadow_length_in, shadow_length_out, num_shadows;
    shadow_length_in = render_context.data->shadow.length;
    num_shadows = shadow_length_in / SHADOW_VERTEX_LENGTH_IN;
    shadow_length_out = 6 * SHADOW_VERTEX_LENGTH_OUT * num_shadows;

    params = (ComputeShaderParams) {
        .compute_shader = SHADER_PROGRAM_SHADOW_COMP,
        .num_objects = num_shadows,
        .object_length_in = shadow_length_in,
        .object_length_out = shadow_length_out,
        .object_buffer = render_context.data->shadow.buffer,
        .output_buffer = shadow_buffers.vbo,
        .output_buffer_capacity_ptr = &shadow_buffers.vbo_capacity
    };

    execute_compute_shader(&params);

    shader_use(SHADER_PROGRAM_SHADOW);
    glBindVertexArray(shadow_buffers.vao);
    glDrawArrays(GL_TRIANGLES, 0, 6 * num_shadows);
}

void game_render_init(void)
{
    log_write(INFO, "Creating game buffers...");

    pthread_mutex_init(&render_context.mutex, NULL);
    render_context.data = st_calloc(1, sizeof(RenderData));
    render_context.data_swap = st_calloc(1, sizeof(RenderData));

    render_context.data->update_tile_buffer = true;
    render_context.data->update_wall_buffer = true;
    render_context.data->update_parstacle_buffer = true;
    render_context.data->update_obstacle_buffer = true;
    render_context.data_swap->update_tile_buffer = true;
    render_context.data_swap->update_wall_buffer = true;
    render_context.data_swap->update_parstacle_buffer = true;
    render_context.data_swap->update_obstacle_buffer = true;

    glGenBuffers(1, &game_time_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, game_time_ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GLdouble), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, UBO_INDEX_GAME_TIME, game_time_ubo);

    glGenVertexArrays(1, &quad_buffers.vao);
    glGenBuffers(1, &quad_buffers.vbo);
    glGenVertexArrays(1, &tile_buffers.vao);
    glGenBuffers(1, &tile_buffers.vbo);
    glGenVertexArrays(1, &wall_buffers.vao);
    glGenBuffers(1, &wall_buffers.vbo);
    glGenVertexArrays(1, &entity_buffers.vao);
    glGenBuffers(1, &entity_buffers.vbo);
    glGenVertexArrays(1, &projectile_buffers.vao);
    glGenBuffers(1, &projectile_buffers.vbo);
    glGenVertexArrays(1, &obstacle_buffers.vao);
    glGenBuffers(1, &obstacle_buffers.vbo);
    glGenVertexArrays(1, &parstacle_buffers.vao);
    glGenBuffers(1, &parstacle_buffers.vbo);
    glGenVertexArrays(1, &particle_buffers.vao);
    glGenBuffers(1, &particle_buffers.vbo);
    glGenVertexArrays(1, &parjicle_buffers.vao);
    glGenBuffers(1, &parjicle_buffers.vbo);
    glGenVertexArrays(1, &shadow_buffers.vao);
    glGenBuffers(1, &shadow_buffers.vbo);
    glGenBuffers(1, &comp_buffers.in);
    glGenBuffers(1, &comp_buffers.out);

    f32 quad_data[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
    };

    glBindVertexArray(quad_buffers.vao);
    glBindBuffer(GL_ARRAY_BUFFER, quad_buffers.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_data), quad_data, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(tile_buffers.vao);
    glBindBuffer(GL_ARRAY_BUFFER, quad_buffers.vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, tile_buffers.vbo);
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

    glBindVertexArray(projectile_buffers.vao);
    glBindBuffer(GL_ARRAY_BUFFER, projectile_buffers.vbo);
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

    glBindVertexArray(parjicle_buffers.vao);
    glBindBuffer(GL_ARRAY_BUFFER, parjicle_buffers.vbo);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(4 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(shadow_buffers.vao);
    glBindBuffer(GL_ARRAY_BUFFER, shadow_buffers.vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    GLuint unit, name;
    unit = texture_get_unit(TEX_SCREEN);
    name = texture_get_name(TEX_SCREEN);
    glGenFramebuffers(1, &render_context.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, render_context.fbo);
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, name);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_width(), window_height(),
            0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, render_context.fbo, 0);
    glGenRenderbuffers(1, &render_context.rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, render_context.rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, window_width(), window_height());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, render_context.rbo);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    unit = texture_get_unit(TEX_SHADOWS);
    name = texture_get_name(TEX_SHADOWS);
    glGenFramebuffers(1, &shadow_buffers.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_buffers.fbo);
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, name);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_width(), window_height(),
            0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shadow_buffers.fbo, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    log_write(INFO, "Created game buffers");
}

void game_render(void)
{
    update_game_time();
    camera_update();

    glBindFramebuffer(GL_FRAMEBUFFER, render_context.fbo);
    glClearColor(1.0f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glStencilFunc(GL_ALWAYS, 1, 0x01);
    glStencilMask(0x01);
    render_walls();

    glStencilFunc(GL_NOTEQUAL, 1, 0x01);
    glStencilMask(0x00);
    render_tiles();

    glStencilFunc(GL_ALWAYS, 0, 0x00);
    glStencilMask(0x01);
    render_obstacles();
    render_parstacles();
    render_entities();
    render_projectiles();
    render_particles();
    render_parjicles();

    glBindFramebuffer(GL_FRAMEBUFFER, shadow_buffers.fbo);
    glClear(GL_COLOR_BUFFER_BIT);

    GLuint loc, unit;
    shader_use(SHADER_PROGRAM_SCREEN);
    loc = shader_get_uniform_location(SHADER_PROGRAM_SCREEN, "screenTex"); 
    unit = texture_get_unit(TEX_SCREEN);
    glUniform1i(loc, unit);
    glBindVertexArray(quad_buffers.vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    render_shadows();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    shader_use(SHADER_PROGRAM_SCREEN);
    unit = texture_get_unit(TEX_SHADOWS);
    glUniform1i(loc, unit);
    glBindVertexArray(quad_buffers.vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void game_render_update_obstacles(void)
{
    render_context.data->update_obstacle_buffer = true;
    render_context.data_swap->update_obstacle_buffer = true;
}

void game_render_update_parstacles(void)
{
    render_context.data->update_parstacle_buffer = true;
    render_context.data_swap->update_parstacle_buffer = true;
}

void game_render_update_tiles(void)
{
    render_context.data->update_tile_buffer = true;
    render_context.data_swap->update_tile_buffer = true;
}

void game_render_update_walls(void)
{
    render_context.data->update_wall_buffer = true;
    render_context.data_swap->update_wall_buffer = true;
}

static void delete_vertex_array(GLuint id)
{
    if (id != 0)
        glDeleteVertexArrays(1, &id);
}

static void delete_buffer(GLuint id)
{
    if (id != 0)
        glDeleteBuffers(1, &id);
}

void game_render_cleanup(void)
{
    log_write(INFO, "Deleting game buffers...");
    pthread_mutex_destroy(&render_context.mutex);
    delete_vertex_array(quad_buffers.vao);
    delete_vertex_array(tile_buffers.vao);
    delete_vertex_array(entity_buffers.vao);
    delete_vertex_array(wall_buffers.vao);
    delete_vertex_array(projectile_buffers.vao);
    delete_vertex_array(parstacle_buffers.vao);
    delete_vertex_array(obstacle_buffers.vao);
    delete_vertex_array(particle_buffers.vao);
    delete_vertex_array(parjicle_buffers.vao);
    delete_buffer(quad_buffers.vbo);
    delete_buffer(wall_buffers.vbo);
    delete_buffer(tile_buffers.vbo);
    delete_buffer(entity_buffers.vbo);
    delete_buffer(projectile_buffers.vbo);
    delete_buffer(obstacle_buffers.vbo);
    delete_buffer(parstacle_buffers.vbo);
    delete_buffer(particle_buffers.vbo);
    delete_buffer(parjicle_buffers.vbo);
    delete_buffer(shadow_buffers.vao);
    delete_buffer(shadow_buffers.vbo);
    delete_buffer(comp_buffers.in);
    delete_buffer(comp_buffers.out);
    delete_buffer(game_time_ubo);
    if (shadow_buffers.fbo != 0)
        glDeleteFramebuffers(1, &shadow_buffers.fbo);
    if (render_context.fbo != 0)
        glDeleteFramebuffers(1, &render_context.fbo);
    if (render_context.rbo != 0)
        glDeleteRenderbuffers(1, &render_context.rbo);

    st_free(render_context.data->projectile.buffer);
    st_free(render_context.data->entity.buffer);
    st_free(render_context.data->tile.buffer);
    st_free(render_context.data->wall.buffer);
    st_free(render_context.data->parstacle.buffer);
    st_free(render_context.data->obstacle.buffer);
    st_free(render_context.data->particle.buffer);
    st_free(render_context.data->parjicle.buffer);
    st_free(render_context.data->shadow.buffer);
    st_free(render_context.data);
    st_free(render_context.data_swap->projectile.buffer);
    st_free(render_context.data_swap->entity.buffer);
    st_free(render_context.data_swap->tile.buffer);
    st_free(render_context.data_swap->wall.buffer);
    st_free(render_context.data_swap->parstacle.buffer);
    st_free(render_context.data_swap->obstacle.buffer);
    st_free(render_context.data_swap->particle.buffer);
    st_free(render_context.data_swap->parjicle.buffer);
    st_free(render_context.data_swap->shadow.buffer);
    st_free(render_context.data_swap);
    log_write(INFO, "Deleted game buffers");
}

