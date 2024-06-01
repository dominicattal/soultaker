#version 460 core

#extension GL_ARB_bindless_texture : require

layout (binding = 0, std430) readonly buffer ssbo {
    uvec2 tex[];
};

layout (std140) uniform Tilt {
    float tilt;
};

in vec2 texCoord;

void main() 
{
    const float PIXEL_SIZE = 0.03;
    float scale = 1.1; // 1 + extra_space * 2;
    vec2 UV = vec2(texCoord.x - 0.5, texCoord.y - 0.5) * scale + 0.5;
    vec4 col = texture(sampler2D(tex[0]), UV);
    if (!(UV.x > 1 || UV.x < 0 || UV.y > 1 || UV.y < 0) && (col.a > 0.1)) {
        gl_FragColor = col;
        return;
    }
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            vec2 dUV = vec2(float(x) * PIXEL_SIZE, float(y) * PIXEL_SIZE);
            vec2 newUV = UV + dUV;
            if (newUV.x > 1 || newUV.x < 0 || newUV.y > 1 || newUV.y < 0)
                continue;
            col = texture(sampler2D(tex[0]), newUV);
            if (col.a > 0.1) {
                gl_FragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
                return;
            }
        }
    }
    discard;
}