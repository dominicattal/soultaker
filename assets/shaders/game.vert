#version 460 core

layout (location = 0) in vec2 aPosition;

layout (std140) uniform Matrices {
    mat4 view;
    mat4 proj;
    float zoom;
};

void main() {
    gl_Position = proj * view * vec4(aPosition.x, 0.0f, aPosition.y, 1.0f);
}
