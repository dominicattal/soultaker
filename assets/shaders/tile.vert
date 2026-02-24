#version 430 core

layout (location = 0) in vec2 aPosition;
layout (location = 1) in vec2 aOffset;
layout (location = 2) in vec4 aTexCoords;
layout (location = 3) in float aLocation;
layout (location = 4) in float aAnimate;

layout (std140) uniform Camera {
    mat4 view;
    mat4 proj;
    float zoom;
    float pitch;
    float yaw;
};

out flat int Location;
out vec2 Position;
out vec4 TexCoords;
out flat int Animate;

void main() {
    gl_Position = proj * view * vec4(aOffset.x + aPosition.x, 0.0f, aOffset.y + aPosition.y, 1.0f);
    Position = aPosition;
    TexCoords = aTexCoords;
    Location = int(round(aLocation));
    Animate = int(round(aAnimate));
}
