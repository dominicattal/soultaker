#version 430

out vec4 Color;

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
    // vec3 position
    // vec4 tex info
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

    int dx, dy;
    vec4 position, color;
    vec2 size;
    position = vec4(data_in[idx], data_in[idx+1], data_in[idx+2], 1.0f);
    position = proj * view * position;
    size.x = data_in[idx+6] / 2.0f / window.aspect_ratio / zoom;
    size.y = data_in[idx+6] / 2.0f / zoom;

    dx = tu[winding[vertex_idx]] * 2 - 1;
    dy = tv[winding[vertex_idx]] * 2 - 1;

    gl_Position = vec4(
                position.x + dx * size.x,
                position.y + dy * size.y,
                position.z,
                position.w
            );
    Color = vec4(data_in[idx+3], data_in[idx+4], data_in[idx+5], 1.0f);
}
