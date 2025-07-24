#version 460

out vec4 FragColor;

uniform sampler2D screenTex;

in float height;
in vec2 ShadowCoords;

void main() {
    float d = distance(ShadowCoords, vec2(0.5f, 0.5f));
    float hsqrd = height * height;
    if (d > 0.5 - hsqrd)
        discard;
    vec4 color = vec4(0.2f, 0.2f, 0.2f, (0.5 - hsqrd - d));
    vec2 tex_coord = vec2(gl_FragCoord.x, gl_FragCoord.y);
    FragColor = texture(screenTex, tex_coord) * color;
}
