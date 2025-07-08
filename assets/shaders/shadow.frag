#version 460

out vec4 FragColor;

in float height;
in vec2 ShadowCoords;

void main() {
    float d = distance(ShadowCoords, vec2(0.5f, 0.5f));
    float hsqrd = height * height;
    if (d > 0.5 - hsqrd)
        discard;
    FragColor = vec4(0.2f, 0.2f, 0.2f, (0.5 - hsqrd - d));
}
