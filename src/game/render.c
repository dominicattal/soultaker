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
} entity_buffers;

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
    GLuint point_buffer;
    GLuint quad_buffer;
} obstacle_buffers;

static struct {
    GLuint vao;
    GLuint point_buffer;
    GLuint quad_buffer;
} particle_buffers;

void game_render_init(void)
{
    glGenVertexArrays(1, &tile_buffers.vao);
    glGenBuffers(1, &tile_buffers.vbo);
    glGenBuffers(1, &tile_buffers.instance_vbo);
    glGenVertexArrays(1, &wall_buffers.vao);
    glGenBuffers(1, &wall_buffers.vbo);
    glGenVertexArrays(1, &entity_buffers.vao);
    glGenBuffers(1, &entity_buffers.point_buffer);
    glGenBuffers(1, &entity_buffers.quad_buffer);
    glGenVertexArrays(1, &proj_buffers.vao);
    glGenBuffers(1, &proj_buffers.point_buffer);
    glGenBuffers(1, &proj_buffers.quad_buffer);
    glGenVertexArrays(1, &obstacle_buffers.vao);
    glGenBuffers(1, &obstacle_buffers.point_buffer);
    glGenBuffers(1, &obstacle_buffers.quad_buffer);
    glGenVertexArrays(1, &particle_buffers.vao);
    glGenBuffers(1, &particle_buffers.point_buffer);
    glGenBuffers(1, &particle_buffers.quad_buffer);

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
    glBindBuffer(GL_ARRAY_BUFFER, entity_buffers.quad_buffer);
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
    glBindBuffer(GL_ARRAY_BUFFER, obstacle_buffers.quad_buffer);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(4 * sizeof(GLfloat)));
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glBindVertexArray(particle_buffers.vao);
    glBindBuffer(GL_ARRAY_BUFFER, particle_buffers.quad_buffer);
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
    shader_use(SHADER_PROGRAM_ENTITY_COMP);
    pthread_mutex_lock(&game_context.data_mutex);
    i32 entity_length = game_context.data.entity_length;
    i32 num_entities = entity_length / 8;
    glUniform1i(shader_get_uniform_location(SHADER_PROGRAM_ENTITY_COMP, "N"), entity_length);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, entity_buffers.point_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, entity_length * sizeof(GLfloat), game_context.data.entity_buffer, GL_STATIC_DRAW);
    pthread_mutex_unlock(&game_context.data_mutex);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, entity_buffers.quad_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 6 * entity_length * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, entity_buffers.point_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, entity_buffers.quad_buffer);
    glDispatchCompute((num_entities + 31) / 32, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    shader_use(SHADER_PROGRAM_ENTITY);
    glBindVertexArray(entity_buffers.vao);
    glDrawArrays(GL_TRIANGLES, 0, 6 * num_entities);
}

static void render_obstacles(void)
{
    shader_use(SHADER_PROGRAM_OBSTACLE_COMP);
    pthread_mutex_lock(&game_context.data_mutex);
    i32 obstacle_length = game_context.data.obstacle_length;
    i32 num_obstacles = obstacle_length / 7;
    glUniform1i(shader_get_uniform_location(SHADER_PROGRAM_OBSTACLE_COMP, "N"), obstacle_length);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, obstacle_buffers.point_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, obstacle_length * sizeof(GLfloat), game_context.data.obstacle_buffer, GL_STATIC_DRAW);
    pthread_mutex_unlock(&game_context.data_mutex);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, obstacle_buffers.quad_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 6 * obstacle_length * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, obstacle_buffers.point_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, obstacle_buffers.quad_buffer);
    glDispatchCompute((num_obstacles + 31) / 32, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    shader_use(SHADER_PROGRAM_OBSTACLE);
    glBindVertexArray(obstacle_buffers.vao);
    glDrawArrays(GL_TRIANGLES, 0, 6 * num_obstacles);
}

static void render_parstacles(void)
{
    shader_use(SHADER_PROGRAM_OBSTACLE_COMP);
    pthread_mutex_lock(&game_context.data_mutex);
    i32 parstacle_length = game_context.data.parstacle_length;
    i32 num_parstacles = parstacle_length / 7;
    glUniform1i(shader_get_uniform_location(SHADER_PROGRAM_OBSTACLE_COMP, "N"), parstacle_length);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, obstacle_buffers.point_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, parstacle_length * sizeof(GLfloat), game_context.data.parstacle_buffer, GL_STATIC_DRAW);
    pthread_mutex_unlock(&game_context.data_mutex);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, obstacle_buffers.quad_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 6 * parstacle_length * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, obstacle_buffers.point_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, obstacle_buffers.quad_buffer);
    glDispatchCompute((num_parstacles + 31) / 32, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    shader_use(SHADER_PROGRAM_OBSTACLE);
    glBindVertexArray(obstacle_buffers.vao);
    glDrawArrays(GL_TRIANGLES, 0, 6 * num_parstacles);
}

static void render_particles(void)
{
    shader_use(SHADER_PROGRAM_PARTICLE_COMP);
    pthread_mutex_lock(&game_context.data_mutex);
    i32 particle_length = game_context.data.particle_length;
    i32 num_particles = particle_length / 7;
    glUniform1i(shader_get_uniform_location(SHADER_PROGRAM_PARTICLE_COMP, "N"), particle_length);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particle_buffers.point_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, particle_length * sizeof(GLfloat), game_context.data.particle_buffer, GL_STATIC_DRAW);
    pthread_mutex_unlock(&game_context.data_mutex);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particle_buffers.quad_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 6 * particle_length * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particle_buffers.point_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, particle_buffers.quad_buffer);
    glDispatchCompute((num_particles + 31) / 32, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

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
    glBufferData(GL_SHADER_STORAGE_BUFFER, 6 * proj_length * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
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
    glDeleteBuffers(1, &entity_buffers.point_buffer);
    glDeleteBuffers(1, &entity_buffers.quad_buffer);
    glDeleteBuffers(1, &proj_buffers.point_buffer);
    glDeleteBuffers(1, &proj_buffers.quad_buffer);
    glDeleteBuffers(1, &obstacle_buffers.point_buffer);
    glDeleteBuffers(1, &obstacle_buffers.quad_buffer);
    glDeleteBuffers(1, &particle_buffers.point_buffer);
    glDeleteBuffers(1, &particle_buffers.quad_buffer);
}

