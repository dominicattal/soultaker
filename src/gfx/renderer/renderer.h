#ifndef RENDERER_H
#define RENDERER_H

#include <glad.h>
#include <glfw.h>
#include "vao.h"
#include "vbo.h"
#include "shader.h"

typedef struct Renderer {
    VAO vao;
    Shader shader;
} Renderer;

extern Renderer renderer;

void renderer_init(void);
void renderer_update(size_t buffer_size);
void renderer_render(void);
void renderer_destroy(void);

/* abstractions */
u32 renderer_uniform_location(char* identifier);

#endif