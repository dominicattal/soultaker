#version 430

layout (std140) uniform Camera {
    mat4 view;
    mat4 proj;
    float zoom;
    float pitch;
    float yaw;
};

layout (std430, binding = 0) readonly buffer Input {
    // float width
    // vec3 pos1
    // vec3 color1
    // vec3 pos2
    // vec3 color2
    float data_in[];
};

uniform int floats_per_vertex;

out vec3 Color;

const int winding[] = {2, 1, 0, 3, 1, 2};
const int tu[] = {0, 1, 0, 1};
const int tv[] = {1, 1, 0, 0};

void main() {
    uint instance_idx = gl_VertexID / 6;
    uint vertex_idx = gl_VertexID % 6;
    uint idx = floats_per_vertex * instance_idx;
    float width = data_in[idx];

    vec3 pos1 = vec3(data_in[idx+1], data_in[idx+2], data_in[idx+3]);
    vec3 pos2 = vec3(data_in[idx+7], data_in[idx+8], data_in[idx+9]);

    vec3 up = vec3(0, 1, 0);
    vec3 forward = pos2 - pos1;
    vec3 right = (2*tu[winding[vertex_idx]]-1) * (width / 2) * normalize(cross(up, forward)); 
    uint data_idx = idx + 1 + 6 * tv[winding[vertex_idx]];

    vec3 pos = vec3(data_in[data_idx], data_in[data_idx+1], data_in[data_idx+2]) + right;
    vec4 position = vec4(pos.x, 0, pos.z, 1.0f);
    position = proj * view * position;
    position.y += pos.y / zoom;
    gl_Position = position;
    Color = vec3(data_in[data_idx+3], data_in[data_idx+4], data_in[data_idx+5]);
}
