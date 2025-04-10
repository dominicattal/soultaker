#version 460

layout (location = 0) in vec4 aPosition;
layout (location = 1) in vec3 aColor;

out vec4 Color;

void main() {
    gl_Position = aPosition;
    Color = vec4(aColor, 1.0f);
}
