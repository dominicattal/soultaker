#include "../game.h"

Line* line_create(void)
{
    return st_malloc(sizeof(Line));
}

void line_destroy(Line* line)
{
    st_free(line);
}
