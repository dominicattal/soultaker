#version 460

layout (location = 0) in vec4 aPosition;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in float aLocation;

out flat int Location;
out vec2 TexCoord;

void main() {
    gl_Position = aPosition;
    TexCoord = aTexCoord;
    Location = int(round(aLocation));
}
