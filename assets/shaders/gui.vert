#version 460 core

layout (location = 0) in vec2 aPosition;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec4 aPosOffset;
layout (location = 3) in vec4 aColor;
layout (location = 4) in vec4 aTexOffset;
layout (location = 5) in float aTexLocation;

layout (std140) uniform Window
{
    int width;
    int height;
} window;

out vec4 Color;

void main() {
    vec2 position;
    position.x = ((aPosOffset.x + aPosition.x * aPosOffset.z) - window.width / 2) / (window.width / 2);
    position.y = ((aPosOffset.y + aPosition.y * aPosOffset.w) - window.height / 2) / (window.height / 2);
    gl_Position = vec4(position, 0.0f, 1.0f);
    Color = aColor;
}
