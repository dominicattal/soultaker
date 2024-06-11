#version 460 core

#extension GL_ARB_bindless_texture : require

layout (location = 0) in vec3 aPos;
layout (location = 1) in float aScale;

layout (std140) uniform Matrices
{
    mat4 view;
    mat4 proj;
};

out VertexData
{
    float scale;
};

void main()
{
    gl_Position = proj * view * vec4(aPos, 1.0f);
    scale = aScale;
}