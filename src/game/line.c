#include "../game.h"

Line* line_create(void)
{
    Line* line = st_malloc(sizeof(Line));
    line->width = 0.1;
    line->lifetime = 5;
    line->pos1 = line->pos2 = \
    line->color1 = line->color2 = vec3_create(0, 0, 0);
    line->use_lifetime = true;
    return line;
}

void line_update(Line* line, f32 dt)
{
    if (line->use_lifetime)
        line->lifetime -= dt;
}

void line_destroy(Line* line)
{
    st_free(line);
}
