#version 430

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aColor;

out vec3 Color;

layout (std140) uniform Camera {
    mat4 view;
    mat4 proj;
    float zoom;
    float pitch;
    float yaw;
};

void main() {
    vec4 position;
    position = vec4(aPosition.x, 0.0f, aPosition.z, 1.0f);
    position = proj * view * position;
    position.y += aPosition.y / zoom;
    gl_Position = position;
    Color = aColor;
}
