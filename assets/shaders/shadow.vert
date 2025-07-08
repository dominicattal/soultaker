#version 460 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec2 aShadowCoords;

layout (std140) uniform Camera {
    mat4 view;
    mat4 proj;
    float zoom;
    float pitch;
    float yaw;
};

out vec2 ShadowCoords;

void main() {
    gl_Position = proj * view * vec4(aPosition, 1.0f);
    ShadowCoords = aShadowCoords;
}
