#version 430

uniform sampler2D textures[16];

out vec4 FragColor;

in vec2 TexCoord;
in vec4 Color;
in flat int TexLocation;

void main() {
    FragColor = texture(textures[TexLocation], TexCoord) * Color;
}
