/*
 * Everything in this file works on
 * the opengl context thread
*/

#include "internal.h"

extern GUIContext gui_context;
static GLuint s_vao, s_vbo, s_instance_vbo;

void gui_render_init(void)
{
    glGenVertexArrays(1, &s_vao);
    glGenBuffers(1, &s_vbo);
    glGenBuffers(1, &s_instance_vbo);

    GLfloat vertices[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
    };

    glBindVertexArray(s_vao);
    glBindBuffer(GL_ARRAY_BUFFER, s_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, s_instance_vbo);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 13 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 13 * sizeof(GLfloat), (void*)(4 * sizeof(GLfloat)));
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 13 * sizeof(GLfloat), (void*)(8 * sizeof(GLfloat)));
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, 13 * sizeof(GLfloat), (void*)(12 * sizeof(GLfloat)));
    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
}

void gui_render(void)
{
    if (gui_context.data.buffer == NULL)
        return;

    glBindVertexArray(s_vao);
    glBindBuffer(GL_ARRAY_BUFFER, s_instance_vbo);

    pthread_mutex_lock(&gui_context.mutex);
    GLsizei instance_count = gui_context.data.instance_count;
    glBufferData(GL_ARRAY_BUFFER, gui_context.data.length * sizeof(GLfloat), gui_context.data.buffer, GL_DYNAMIC_DRAW);
    pthread_mutex_unlock(&gui_context.mutex);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, instance_count);
}

void gui_render_cleanup(void)
{
    glDeleteVertexArrays(1, &s_vao);
    glDeleteBuffers(1, &s_vbo);
    glDeleteBuffers(1, &s_instance_vbo);
}

