#ifndef SHADER_H
#define SHADER_H

#include <glad.h>
#include <glfw.h>
#include <gtype.h>

typedef struct Shader {
    u32 ID;
} Shader;

Shader shader_create(char* vs_path, char* fs_path);
void shader_use(Shader shader);

#endif