#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

layout (std140) uniform Matrices
{
    mat4 view;
    mat4 proj;
};

out float height;
out vec2 texCoord;

void main()
{
    vec4 position = gl_in[0].gl_Position;
    height = position.y;
    position.y = 0.0f;
    gl_Position = proj * view * (position + vec4(-0.5, 0.0, -0.5, 0.0));
    texCoord = vec2(0.0f, 0.0f);
    EmitVertex();
    gl_Position = proj * view * (position + vec4(0.5, 0.0, -0.5, 0.0));
    texCoord = vec2(1.0f, 0.0f);
    EmitVertex();
    gl_Position = proj * view * (position + vec4(-0.5, 0.0, 0.5, 0.0));
    texCoord = vec2(0.0f, 1.0f);
    EmitVertex();
    gl_Position = proj * view * (position + vec4(0.5, 0.0, 0.5, 0.0));
    texCoord = vec2(1.0f, 1.0f);
    EmitVertex();
    EndPrimitive();
}