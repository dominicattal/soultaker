/*
 * Everything in this file
 * runs on the opengl context
 * thread
*/

#include "internal.h"
#include "../renderer.h"

extern GameContext game_context;
static GLuint s_vao, s_vbo;

void game_render_init(void)
{
    glGenVertexArrays(1, &s_vao);
    glGenBuffers(1, &s_vbo);

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
}

void game_render(void)
{
    shader_use(SHADER_PROGRAM_GAME);
    glEnable(GL_DEPTH_TEST);
    glBindVertexArray(s_vao);
    glBindBuffer(GL_ARRAY_BUFFER, s_vbo);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void game_render_cleanup(void)
{
    glDeleteVertexArrays(1, &s_vao);
    glDeleteBuffers(1, &s_vbo);
}

