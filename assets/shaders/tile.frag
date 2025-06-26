#version 460 core

uniform sampler2D textures[16];

out vec4 FragColor;

in flat int Location;
in vec2 Position;
in vec4 TexCoords;
in flat int Animate;

layout (std140) uniform GameTime {
    float game_time;
};

void main() {
    float t, u, v, du, dv;
    vec2 TexOffset;
    int idx_u, idx_v;
    float offset_u[] = {0, -1, 1, 0};
    float offset_v[] = {0, -1, 1, 0};
    u = TexCoords.x;
    v = TexCoords.y;
    du = TexCoords.z;
    dv = TexCoords.w;
    t = game_time - int(game_time);
    idx_u = Animate & 3;
    idx_v = (Animate>>2) & 3;
    TexOffset.x = du * mod(Position.x + offset_u[idx_u] * t, 1.0f);
    TexOffset.y = dv * mod(Position.y + offset_v[idx_v] * t, 1.0f);

    vec2 TexCoord;
    TexCoord.x = u + TexOffset.x;
    TexCoord.y = v + TexOffset.y;

    FragColor = texture(textures[Location], TexCoord);
}
