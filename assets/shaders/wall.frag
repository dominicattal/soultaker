#version 460

uniform sampler2D textures[16];

out vec4 FragColor;

in flat float depthValue;

void main() {
    gl_FragDepth = depthValue;
    FragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);
}
