#version 460 core

layout (location = 0) in vec3 aPosition;

layout (std140) uniform Matrices {
    mat4 view;
    mat4 proj;
    float zoom;
};

void main() {
    gl_Position = proj * view * vec4(aPosition, 1.0f);
}
