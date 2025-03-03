/*
 * Everything in this file
 * runs on the opengl context
 * thread
*/

#include "internal.h"
#include "../renderer.h"

extern GameContext game_context;
static GLuint s_vao, s_vbo, s_ent_vao, s_ent_vbo_model, s_ent_vbo, s_ent_ssbo;

void game_render_init(void)
{
    glGenVertexArrays(1, &s_vao);
    glGenBuffers(1, &s_vbo);
    glGenVertexArrays(1, &s_ent_vao);
    glGenBuffers(1, &s_ent_vbo);
    glGenBuffers(1, &s_ent_ssbo);
    glGenBuffers(1, &s_ent_vbo_model);

    f32 test_data[] = {
        0.0f, 0.0f, 0.0f,
        20.0f, 0.0f, 0.0f,
        20.0f, 0.0f, 20.0f, 
        0.0f, 0.0f, 0.0f,
        20.0f, 0.0f, 20.0f, 
        0.0f, 0.0f, 20.0f
    };

    glBindVertexArray(s_vao);
    glBindBuffer(GL_ARRAY_BUFFER, s_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(test_data), test_data, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(s_ent_vao);
    glBindBuffer(GL_ARRAY_BUFFER, s_ent_vbo);
    glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, s_ent_ssbo);
    glBufferData(GL_ARRAY_BUFFER, 4 * 6 * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
}

void game_render(void)
{
    shader_use(SHADER_PROGRAM_GAME);
    glEnable(GL_DEPTH_TEST);
    glBindVertexArray(s_vao);
    glBindBuffer(GL_ARRAY_BUFFER, s_vbo);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    if (game_context.player == NULL)
        return;

    shader_use(SHADER_PROGRAM_ENTITY_COMP);
    glUniform1i(shader_get_uniform_location(SHADER_PROGRAM_ENTITY_COMP, "N"), 1);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, s_ent_vbo);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 3 * sizeof(GLfloat), &game_context.player->position);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, s_ent_vbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, s_ent_ssbo);
    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    GLfloat* buf = malloc(6 * 4 * sizeof(GLfloat));
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, s_ent_ssbo);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 6 * 4 * sizeof(GLfloat), buf);
    vec3_print(game_context.player->position);
    for (i32 i = 0; i < 6 * 4; i++)
        printf((i % 4 == 3) ? "%f\n" : "%f ", buf[i]);
    puts("");
    free(buf);

    shader_use(SHADER_PROGRAM_ENTITY);
    glBindVertexArray(s_ent_vao);
    //glBindBuffer(GL_VERTEX_ARRAY, s_ent_ssbo);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void game_render_cleanup(void)
{
    glDeleteVertexArrays(1, &s_vao);
    glDeleteVertexArrays(1, &s_ent_vao);
    glDeleteBuffers(1, &s_vbo);
    glDeleteBuffers(1, &s_ent_vbo);
    glDeleteBuffers(1, &s_ent_ssbo);
    glDeleteBuffers(1, &s_ent_vbo_model);
}

