#version 460

#define PI 3.141592653589

layout (std140) uniform Minimap {
    vec2 center;
    float yaw;
    float zoom;
    float ar;
};

layout (std430, binding = 0) readonly buffer Input {
    // vec2 position
    // float radius
    // float size
    float data_in[];
};

uniform int floats_per_vertex;
out vec2 TexCoord;

const int winding[] = {0, 1, 2, 1, 3, 2};
const int tu[] = {0, 1, 0, 1};
const int tv[] = {0, 0, 1, 1};

void main() {
    uint instance_idx = gl_VertexID / 6;
    uint vertex_idx = gl_VertexID % 6;
    uint idx = floats_per_vertex * instance_idx;

    int dx, dy;
    vec2 position = vec2(data_in[idx] - center.x, -data_in[idx+1] + center.y);
    vec2 pos;
    float r = data_in[idx+2];
    float size = data_in[idx+4];
    float s = sin(yaw-PI/2);
    float c = cos(yaw-PI/2);

    dx = tu[winding[vertex_idx]] * 2 - 1;
    dy = tv[winding[vertex_idx]] * 2 - 1;
    position.x += dx * r;
    position.y += dy * r;

    gl_Position = vec4(
                (position.x * c - position.y * s) / zoom,
                (position.x * s + position.y * c) / zoom * ar,
                0.0f,
                1.0f
            );
    TexCoord = vec2(tu[winding[vertex_idx]], tv[winding[vertex_idx]]);
}
