#version 460

out flat int Location;
out vec2 TexCoord;

layout (std140) uniform Camera {
    mat4 view;
    mat4 proj;
    float zoom;
    float pitch;
    float yaw;
};

layout (std140) uniform Window {
    int width;
    int height;
    float aspect_ratio; 
} window;

layout (std430, binding = 0) readonly buffer Input {
    // vec2 position
    // float size
    // vec4 tex info
    // float location
    float data_in[];
};

uniform int floats_per_vertex;

const int winding[] = {0, 1, 2, 2, 1, 3};
const int tu[] = {0, 1, 0, 1};
const int tv[] = {1, 1, 0, 0};

void main() {
    uint instance_idx = gl_VertexID / 6;
    uint vertex_idx = gl_VertexID % 6;
    uint idx = floats_per_vertex * instance_idx;
    vec4 position;
    position = vec4(data_in[idx], 0.0f, data_in[idx+1], 1.0f);
    position = proj * view * position;

    vec2 size;
    float u, v, du, dv;
    int dx, dy;
    size.x = data_in[idx+2] / 2.0f / window.aspect_ratio / zoom;
    size.y = data_in[idx+2] / zoom;
    u = data_in[idx+3];
    v = data_in[idx+4];
    du = data_in[idx+5];
    dv = data_in[idx+6];

    dx = tu[winding[vertex_idx]] * 2 - 1;
    dy = 1 - tv[winding[vertex_idx]];

    gl_Position = vec4(
                position.x + dx * size.x,
                position.y + dy * size.y,
                position.z,
                position.w
            );
    TexCoord = vec2(u + tu[winding[vertex_idx]] * du,
                    v + tv[winding[vertex_idx]] * dv);
    Location = int(round(data_in[idx+7]));
}
