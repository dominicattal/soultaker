#version 460 core

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

layout (std140) uniform AspectRatio
{
    float ar;
};

layout (std140) uniform Zoom
{
    float zoom;
};

out vec2 texCoord;

in flat float scale[];

void main() {
    vec4 position = gl_in[0].gl_Position;
    vec2 offset;

    // bottom left
    offset = zoom * scale[0] * vec2(-0.5f * ar, -0.5f);
    texCoord = vec2(0.0f, 1.0f);
    gl_Position = position + vec4(offset, 0.0f, 0.0f);
    EmitVertex();

    // bottom right
    offset = zoom * scale[0] * vec2(0.5f * ar, -0.5f);
    texCoord = vec2(1.0f, 1.0f);
    gl_Position = position + vec4(offset, 0.0f, 0.0f);
    EmitVertex();

    // top left
    offset = zoom * scale[0] * vec2(-0.5f * ar, 0.5f);
    texCoord = vec2(0.0f, 0.0f);
    gl_Position = position + vec4(offset, 0.0f, 0.0f);
    EmitVertex();

    // top right
    offset = zoom * scale[0] * vec2(0.5f * ar, 0.5f);
    texCoord = vec2(1.0f, 0.0f);
    gl_Position = position + vec4(offset, 0.0f, 0.0f);
    EmitVertex();
    
    EndPrimitive();
}