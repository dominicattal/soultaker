#version 460

#define PI 3.141592653589

layout (location = 0) in vec2 inPosition;
layout (location = 1) in vec2 inOffset;
layout (location = 2) in vec2 inSize;
layout (location = 3) in vec3 inColor;

layout (std140) uniform Minimap {
    vec2 center;
    float yaw;
    float zoom;
    float ar;
};

out vec3 color;

void main()
{
    vec2 position = vec2(inOffset.x - center.x, -inOffset.y + center.y);
    float s = sin(yaw-PI/2);
    float c = cos(yaw-PI/2);
    position.x += inPosition.x * inSize.x;
    position.y += (inPosition.y - 1) * inSize.y;
    gl_Position = vec4(
                (position.x * c - position.y * s) / zoom,
                (position.x * s + position.y * c) / zoom * ar,
                0.0f,
                1.0f
            );
    color = inColor;
}
