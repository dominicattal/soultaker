#version 460

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in float aLocation;

layout (std140) uniform Matrices {
    mat4 view;
    mat4 proj;
    float zoom;
};

void main() {
    gl_Position = proj * view * vec4(aPosition, 1.0f);
}
