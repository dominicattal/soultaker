#include "internal.h"
#include "../renderer.h"
#include <math.h>

extern GameContext game_context;

void wall_init(void)
{
    game_context.walls = list_create();
    game_context.free_walls = list_create();
}

void wall_clear(void)
{
    if (game_context.walls == NULL)
        return;
    while (game_context.walls->length > 0)
        wall_destroy(list_remove(game_context.walls, 0));
    list_clear(game_context.free_walls);
}

Wall* wall_create(vec2 position, f32 height)
{
    Wall* wall = st_malloc(sizeof(Wall));
    wall->position = position;
    wall->size = vec2_create(1, 1);
    wall->height = height;
    wall->top_tex = texture_get_id("wall_1");
    wall->side_tex = texture_get_id("wall_2");
    list_append(game_context.walls, wall);
    return wall;
}

void wall_update_free_walls(void)
{
    Wall* wall;
    i32 x, z;
    list_clear(game_context.free_walls);
    for (i32 i = 0; i < game_context.walls->length; i++) {
        wall = list_get(game_context.walls, i);
        x = round(wall->position.x);
        z = round(wall->position.z);
        if (map_get_wall(x, z) != wall)
            list_append(game_context.free_walls, wall);
    }
}

void wall_destroy(Wall* wall)
{
    st_free(wall);
}

void wall_cleanup(void)
{
    if (game_context.walls == NULL)
        return;
    for (i32 i = 0; i < game_context.walls->length; i++)
        wall_destroy(list_get(game_context.walls, i));
    list_destroy(game_context.walls);
    list_destroy(game_context.free_walls);
}

