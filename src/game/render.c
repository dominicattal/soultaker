/*
 * Everything in this file
 * runs on the opengl context
 * thread
*/

#include "internal.h"
#include "../renderer.h"

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
    i32 num_tiles = tile_length / 7;
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
    #define FLOATS_PER_VERTEX_IN  8
    #define FLOATS_PER_VERTEX_OUT 7
    i32 entity_length_in, entity_length_out, num_entities;
    entity_length_in = game_context.data.entity_length;
    num_entities = entity_length_in / FLOATS_PER_VERTEX_IN;
    entity_length_out = 6 * FLOATS_PER_VERTEX_OUT * num_entities;
    #undef FLOATS_PER_VERTEX_IN
    #undef FLOATS_PER_VERTEX_OUT

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
    #define FLOATS_PER_VERTEX_IN  7
    #define FLOATS_PER_VERTEX_OUT 7
    i32 obstacle_length_in, obstacle_length_out, num_obstacles;
    obstacle_length_in = game_context.data.obstacle_length;
    num_obstacles = obstacle_length_in / FLOATS_PER_VERTEX_IN;
    obstacle_length_out = 6 * FLOATS_PER_VERTEX_OUT * num_obstacles;
    #undef FLOATS_PER_VERTEX_IN
    #undef FLOATS_PER_VERTEX_OUT

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
}

static void render_particles(void)
{
    #define FLOATS_PER_VERTEX_IN  7
    #define FLOATS_PER_VERTEX_OUT 7
    i32 particle_length_in, particle_length_out, num_particles;
    particle_length_in = game_context.data.particle_length;
    num_particles = particle_length_in / FLOATS_PER_VERTEX_IN;
    particle_length_out = 6 * FLOATS_PER_VERTEX_OUT * num_particles;
    #undef FLOATS_PER_VERTEX_IN
    #undef FLOATS_PER_VERTEX_OUT

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
    shader_use(SHADER_PROGRAM_PROJECTILE_COMP);
    pthread_mutex_lock(&game_context.data_mutex);
    i32 proj_length = game_context.data.proj_length;
    i32 num_projs = proj_length / 8;
    glUniform1i(shader_get_uniform_location(SHADER_PROGRAM_PROJECTILE_COMP, "N"), proj_length);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, proj_buffers.point_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, proj_length * sizeof(GLfloat), game_context.data.proj_buffer, GL_STATIC_DRAW);
    pthread_mutex_unlock(&game_context.data_mutex);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, proj_buffers.quad_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 100 * num_projs * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, proj_buffers.point_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, proj_buffers.quad_buffer);
    glDispatchCompute((num_projs + 31) / 32, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    shader_use(SHADER_PROGRAM_PROJECTILE);
    glBindVertexArray(proj_buffers.vao);
    glDrawArrays(GL_TRIANGLES, 0, 6 * num_projs);
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
    glDeleteBuffers(1, &particle_buffers.vbo);
    glDeleteBuffers(1, &comp_buffers.in);
    glDeleteBuffers(1, &comp_buffers.out);
}

