#ifndef VAO_H
#define VAO_H

#include <glad.h>
#include <glfw.h>
#include <gtype.h>
#include "vbo.h"

typedef struct VAO {
    u32 ID;
    VBO vbo, ebo;
    u32 index_count, index_size;
    u32* indices;
} VAO;

VAO vao_create(u32 index_count, u32 index_size, u32* indices);
void vao_update(VAO* vao, GLenum type, size_t data_size, void* data);
void vao_destroy(VAO vao);
void vao_draw(VAO vao);

#endif