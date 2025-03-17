#version 460

uniform sampler2D textures[16];

out vec4 FragColor;

void main() {
    FragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);
}
