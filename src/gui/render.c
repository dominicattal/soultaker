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
        0.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 0.0f,
    };

/*    static GLfloat dummy_data[] = {
        0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f
    };

    gui_context.data.buffer = dummy_data;
    gui_context.data.length = 13;
    gui_context.data.capacity = 13;
*/
    glBindVertexArray(s_vao);
    glBindBuffer(GL_ARRAY_BUFFER, s_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, s_instance_vbo);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 13 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 13 * sizeof(GLfloat), (void*)(4 * sizeof(GLfloat)));
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 13 * sizeof(GLfloat), (void*)(8 * sizeof(GLfloat)));
    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, 13 * sizeof(GLfloat), (void*)(12 * sizeof(GLfloat)));
    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);
}

void gui_render(void)
{
    if (gui_context.data.buffer == NULL)
        return;

    glBindVertexArray(s_vao);
    glBindBuffer(GL_ARRAY_BUFFER, s_instance_vbo);

    pthread_mutex_lock(&gui_context.mutex);
    glBufferData(GL_ARRAY_BUFFER, gui_context.data.length * sizeof(GLfloat), gui_context.data.buffer, GL_DYNAMIC_DRAW);
    pthread_mutex_unlock(&gui_context.mutex);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, 1);
}

void gui_render_cleanup(void)
{
    glDeleteVertexArrays(1, &s_vao);
    glDeleteBuffers(1, &s_vbo);
    glDeleteBuffers(1, &s_instance_vbo);
}

