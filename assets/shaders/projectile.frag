#version 460

uniform sampler2D textures[16];

out vec4 FragColor;

in flat int Location;
in vec2 TexCoord;

void main() {
    FragColor = texture(textures[Location], TexCoord);
    if (FragColor.a < 0.1)
        discard;
}
