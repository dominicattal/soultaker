#include "../game.h"
#include "../renderer.h"
#include <math.h>

extern GameContext game_context;

void wall_init(void)
{
}

Wall* wall_create(vec2 position, f32 height, u32 minimap_color)
{
    Wall* wall = st_malloc(sizeof(Wall));
    wall->position = position;
    wall->minimap_color = minimap_color;
    wall->size = vec2_create(1, 1);
    wall->height = height;
    wall->top_tex = texture_get_id("wall_1");
    wall->side_tex = texture_get_id("wall_2");
    wall->flags = 0;
    wall_set_flag(wall, WALL_FLAG_ACTIVE, true);
    return wall;
}

void wall_set_flag(Wall* wall, WallFlagEnum flag, bool val)
{
    wall->flags = (wall->flags & ~(1<<flag)) | (val<<flag);
}

bool wall_get_flag(Wall* wall, WallFlagEnum flag)
{
    return (wall->flags >> flag) & 1;
}

void wall_destroy(Wall* wall)
{
    st_free(wall);
}

void wall_cleanup(void)
{
}

