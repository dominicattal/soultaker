#include "internal.h"
#include "../renderer.h"

extern GameContext game_context;

void wall_init(void)
{
    game_context.walls = list_create();
    wall_create(vec2_create(0, 0), 1.5f);
    wall_create(vec2_create(3, 0), 1.5f);
}

Wall* wall_create(vec2 position, f32 height)
{
    Wall* wall = malloc(sizeof(Wall));
    wall->position = position;
    wall->height = height;
    wall->top_tex = TEX_WALL_1;
    wall->side_tex = TEX_WALL_2;
    list_append(game_context.walls, wall);
    return wall;
}

void wall_destroy(Wall* wall)
{
    free(wall);
}

void wall_cleanup(void)
{
    if (game_context.walls != NULL)
        list_destroy(game_context.walls);
}

