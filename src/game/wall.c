#include "internal.h"
#include "../renderer.h"

extern GameContext game_context;

void wall_init(void)
{
    game_context.walls = list_create();
    wall_create(vec2_create(-3, -3), 1.5f);
    wall_create(vec2_create(-5, -3), 1.5f);
    for (i32 i = -101; i <= 101; i++) {
        wall_create(vec2_create(i, -101), 1.5f);
        wall_create(vec2_create(101, i), 1.5f);
        wall_create(vec2_create(-i-1, 101), 1.5f);
        wall_create(vec2_create(-101, -i-1), 1.5f);
    }
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
    if (game_context.walls == NULL)
        return;
    for (i32 i = 0; i < game_context.walls->length; i++)
        free(list_get(game_context.walls, i));
    list_destroy(game_context.walls);
}

