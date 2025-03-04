#version 460

layout (location = 0) in vec4 aPosition;

layout (std140) uniform Matrices {
    mat4 view;
    mat4 proj;
    float zoom;
};

void main() {
    gl_Position = aPosition;
    gl_PointSize = 6;
}
