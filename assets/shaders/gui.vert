#version 430

layout (location = 0) in vec2 aPosition;
layout (location = 1) in vec4 aPosOffset;
layout (location = 2) in vec4 aColor;
layout (location = 3) in vec4 aTexOffset;
layout (location = 4) in float aTexLocation;

layout (std140) uniform Window {
    int width;
    int height;
    float aspect_ratio;
} window;

out vec2 TexCoord;
out vec4 Color;
out flat int TexLocation;

void main() {
    vec2 position;
    position.x = ((aPosOffset.x + aPosition.x * aPosOffset.z) - window.width / 2) / (window.width / 2);
    position.y = ((aPosOffset.y + aPosition.y * aPosOffset.w) - window.height / 2) / (window.height / 2);
    gl_Position = vec4(position, 0.0f, 1.0f);
    TexCoord.x = (1 - aPosition.x) * aTexOffset.x + aPosition.x * aTexOffset.z;
    TexCoord.y = (1 - aPosition.y) * aTexOffset.w + aPosition.y * aTexOffset.y;
    TexLocation = int(round(aTexLocation));
    Color = aColor / 255;
}
