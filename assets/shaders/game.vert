#version 460 core

layout (location = 0) in vec2 aPosition;
layout (location = 1) in vec2 aOffset;
layout (location = 2) in vec4 aTexCoords;
layout (location = 3) in float aLocation;

layout (std140) uniform Matrices {
    mat4 view;
    mat4 proj;
    float zoom;
};

out flat int Location;
out vec2 TexCoord;

void main() {
    gl_Position = proj * view * vec4(aOffset.x + aPosition.x, 0.0f, aOffset.y + aPosition.y, 1.0f);
    TexCoord = vec2(aTexCoords.x + aTexCoords.z * aPosition.x, aTexCoords.y + aTexCoords.w * aPosition.y);
    Location = int(round(aLocation));
}
