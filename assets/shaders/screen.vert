#version 430

layout (location = 0) in vec2 aPos;

out vec2 tex_coords;

void main()
{
    gl_Position = vec4(2*aPos.x-1,2*aPos.y-1, 0.0f, 1.0f);
    tex_coords = aPos;
}
