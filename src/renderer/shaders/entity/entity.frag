#version 330 core
// http://www.geoffprewett.com/blog/software/opengl-outline/index.html

in vec2 texCoord;

uniform sampler2D entity;
void main()
{
    const float PIXEL_SIZE = 0.02;
    float a = 1.1; // 1 + buffer * 2;
    vec2 UV = vec2(texCoord.x - 0.5, texCoord.y - 0.5) * a + 0.5;
    vec4 col = texture(entity, UV);
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
            col = texture(entity, newUV);
            if (col.a > 0.1) {
                gl_FragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
                return;
            }
        }
    }
    discard;
}