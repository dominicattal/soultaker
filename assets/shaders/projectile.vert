#version 460

#define SQRT2 1.41421356237
#define PI 3.141592653589

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
    // float facing
    // float rotation
    // vec4 tex_info
    // float location
    float data_in[];
};

uniform int floats_per_vertex;

const int winding[] = {0, 1, 2, 2, 1, 3};
const float rot_offset[] = {-PI, - PI / 2, PI / 2, 0};
const int tu[] = {0, 1, 0, 1};
const int tv[] = {1, 1, 0, 0};

void main() {
    uint instance_idx = gl_VertexID / 6;
    uint vertex_idx = gl_VertexID % 6;
    uint idx = floats_per_vertex * instance_idx;

    vec4 position;
    vec2 offset;
    float size, facing, rotation, u, v, du, dv;
    float drot, theta;
    
    position = vec4(data_in[idx], 0.0f, data_in[idx+2], 1.0f);
    position = proj * view * position;
    position.y += data_in[idx+1] / zoom;
    size = abs(data_in[idx+3]);
    facing = data_in[idx+4];
    rotation = data_in[idx+5];
    u = data_in[idx+6];
    v = data_in[idx+7];
    du = data_in[idx+8];
    dv = data_in[idx+9];

    drot = -facing+yaw;
    drot = rotation + atan(tan(drot) / cos(PI / 2 + pitch)) + (cos(drot) > 0 ? 0 : PI) + (size < 0 ? 0 : PI / 4);

    theta = drot + rot_offset[winding[vertex_idx]];
    offset = size / zoom / SQRT2 * vec2(cos(theta) / window.aspect_ratio, sin(theta));

    gl_Position = vec4(
                position.x + offset.x,
                position.y + offset.y,
                position.z,
                position.w
            );
    TexCoord = vec2(u + tu[winding[vertex_idx]] * du,
                    v + tv[winding[vertex_idx]] * dv);
    Location = int(round(data_in[idx+10]));
}
