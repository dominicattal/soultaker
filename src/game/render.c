    /*
 * Everything in this file
 * runs on the opengl context
 * thread
*/

#include "internal.h"
#include "../renderer.h"

extern GameContext game_context;
static GLuint s_ent_vao, s_ent_vbo_model, s_ent_vbo, s_ent_ssbo;

static struct {
    GLuint vao;
    GLuint vbo;
    GLuint instance_vbo;
} tile_buffers;

void game_render_init(void)
{
    glGenVertexArrays(1, &tile_buffers.vao);
    glGenBuffers(1, &tile_buffers.vbo);
    glGenBuffers(1, &tile_buffers.instance_vbo);
    glGenVertexArrays(1, &s_ent_vao);
    glGenBuffers(1, &s_ent_vbo);
    glGenBuffers(1, &s_ent_ssbo);
    glGenBuffers(1, &s_ent_vbo_model);

    f32 tile_data[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
    };

    glBindVertexArray(tile_buffers.vao);
    glBindBuffer(GL_ARRAY_BUFFER, tile_buffers.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tile_data), tile_data, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, tile_buffers.instance_vbo);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);

    glBindVertexArray(s_ent_vao);
    glBindBuffer(GL_ARRAY_BUFFER, s_ent_ssbo);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
}

void game_render(void)
{
    shader_use(SHADER_PROGRAM_GAME);
    glBindVertexArray(tile_buffers.vao);
    glBindBuffer(GL_ARRAY_BUFFER, tile_buffers.instance_vbo);
    pthread_mutex_lock(&game_context.data_mutex);
    i32 tile_length = game_context.data.tile_length;
    i32 num_tiles = tile_length / 2;
    glBufferData(GL_ARRAY_BUFFER, tile_length * sizeof(GLfloat), game_context.data.tile_buffer, GL_STATIC_DRAW);
    pthread_mutex_unlock(&game_context.data_mutex);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 6 * num_tiles);
    
    glEnable(GL_DEPTH_TEST);
    shader_use(SHADER_PROGRAM_ENTITY_COMP);
    pthread_mutex_lock(&game_context.data_mutex);
    i32 entity_length = game_context.data.entity_length;
    i32 num_entities = entity_length / 4;
    glUniform1i(shader_get_uniform_location(SHADER_PROGRAM_ENTITY_COMP, "N"), entity_length);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, s_ent_vbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, entity_length * sizeof(GLfloat), game_context.data.entity_buffer, GL_STATIC_DRAW);
    pthread_mutex_unlock(&game_context.data_mutex);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, s_ent_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 6 * entity_length * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, s_ent_vbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, s_ent_ssbo);
    glDispatchCompute((num_entities + 31) / 32, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    shader_use(SHADER_PROGRAM_ENTITY);
    glBindVertexArray(s_ent_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6 * num_entities);
}

void game_render_cleanup(void)
{
    glDeleteVertexArrays(1, &tile_buffers.vao);
    glDeleteVertexArrays(1, &s_ent_vao);
    glDeleteBuffers(1, &tile_buffers.vbo);
    glDeleteBuffers(1, &tile_buffers.instance_vbo);
    glDeleteBuffers(1, &s_ent_vbo);
    glDeleteBuffers(1, &s_ent_ssbo);
    glDeleteBuffers(1, &s_ent_vbo_model);
}

