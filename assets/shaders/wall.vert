#version 460

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in float aLocation;
layout (location = 3) in vec2 aCenter;

layout (std140) uniform Camera {
    mat4 view;
    mat4 proj;
    float zoom;
    float pitch;
    float yaw;
};

out flat int Location;
out vec2 TexCoord;
out flat float depthValue;

void main() {
    gl_Position = proj * view * vec4(aPosition, 1.0f);
    vec4 center = proj * view * vec4(aCenter.x, 0.0f, aCenter.y, 1.0);
    depthValue = 0.5 + 0.5 * center.z / center.w;
    Location = int(round(aLocation));
    TexCoord = aTexCoord;
}
