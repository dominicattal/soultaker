#version 430 core

out flat float height;
out vec2 ShadowCoords;

layout (std140) uniform Camera {
    mat4 view;
    mat4 proj;
    float zoom;
    float pitch;
    float yaw;
};

layout (std430, binding = 0) readonly buffer Input {
    // vec3 position
    // float size
    float data_in[];
};

uniform int floats_per_vertex;

const int winding[] = {0, 1, 2, 2, 1, 3};
const int tu[] = {0, 1, 0, 1};
const int tv[] = {0, 0, 1, 1};

void main() {
    uint instance_idx = gl_VertexID / 6;
    uint vertex_idx = gl_VertexID % 6;
    uint idx = floats_per_vertex * instance_idx;
    vec4 position = vec4(data_in[idx], 0.0f, data_in[idx+2], 1.0f);
    float size = data_in[idx+3];
    int dx, dz;

    dx = tu[winding[vertex_idx]] * 2 - 1;
    dz = tv[winding[vertex_idx]] * 2 - 1;

    position.x += dx * size / 2;
    position.z += dz * size / 2;

    gl_Position = proj * view * position;
    height = data_in[idx+1];
    ShadowCoords = vec2(tu[winding[vertex_idx]], tv[winding[vertex_idx]]);
}
