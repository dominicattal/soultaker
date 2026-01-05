#version 430

out vec4 FragColor;

in vec2 TexCoord;
in vec3 Color;

void main() {
    float x = TexCoord.x - 0.5;
    float y = TexCoord.y - 0.5;
    if (x*x + y*y > 0.5*0.5)
        discard;
    FragColor = vec4(Color , 1.0f);
}
