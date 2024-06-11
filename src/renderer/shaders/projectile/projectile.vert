#version 460 core

#extension GL_ARB_bindless_texture : require

layout (location = 0) in vec3 aPos;
layout (location = 1) in float aScale;
layout (location = 2) in float aRotation;

out float projectile_rotation;

layout (std140) uniform Matrices
{
    mat4 view;
    mat4 proj;
};

layout (std140) uniform Zoom
{
    float zoom;
};

out VertexData
{
    float scale;
    float projectileRotation;
};

void main()
{
    gl_Position = proj * view * vec4(aPos.x, 0.0, aPos.z, 1.0f);
    gl_Position.y += zoom * aPos.y;
    scale = aScale;
    projectileRotation = aRotation;
}