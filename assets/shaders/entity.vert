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
    // vec3 position
    // float size
    // vec4 tex_info
    // float location
    // vec2 pivot
    // vec2 stretch
    float data_in[];
};

uniform int floats_per_vertex;

int winding[] = {0, 1, 2, 2, 1, 3};
int tu[] = {0, 1, 0, 1};
int tv[] = {1, 1, 0, 0};

void main() {
    uint instance_idx = gl_VertexID / 6;
    uint vertex_idx = gl_VertexID % 6;
    uint idx = floats_per_vertex * instance_idx;

    vec4 position;
    vec2 pivot, stretch, offset, size;
    float u, v, du, dv;
    int dx, dy;

    position = vec4(data_in[idx], 0.0f, data_in[idx+2], 1.0f);
    position = proj * view * position;
    position.y += data_in[idx+1] / zoom;

    size.x = data_in[idx+3] / 2.0f / window.aspect_ratio / zoom;
    size.y = data_in[idx+3] / zoom;
    u = data_in[idx+4];
    v = data_in[idx+5];
    du = data_in[idx+6];
    dv = data_in[idx+7];
    pivot.x = -data_in[idx+9];
    pivot.y = -data_in[idx+10];
    stretch.x = data_in[idx+11];
    stretch.y = data_in[idx+12];
    
    dx = tu[winding[vertex_idx]] * 2 - 1;
    dy = 1 - tv[winding[vertex_idx]];

    gl_Position = vec4(
                position.x + dx * size.x * (stretch.x + dx * pivot.x),
                position.y + dy * size.y * (stretch.y + pivot.y),
                position.z,
                position.w
            );
    TexCoord = vec2(u + tu[winding[vertex_idx]] * du,
                    v + tv[winding[vertex_idx]] * dv);
    Location = int(round(data_in[idx+8]));
}
