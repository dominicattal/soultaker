#include "internal.h"
#include "../renderer.h"

extern GameContext game_context;

void wall_init(void)
{
    game_context.walls = list_create();
    Wall* wall = wall_create(vec2_create(-3, -3), 1.5f);
    wall->size = vec2_create(0.5, 0.5);
    wall = wall_create(vec2_create(-1, -1), 1.5f);
    wall->size = vec2_create(1, 0);
    wall_create(vec2_create(-5, -3), 1.5f);
    i32 a = 11;
    for (i32 i = -a; i <= a; i++) {
        wall_create(vec2_create(i, -a), 1.5f);
        wall_create(vec2_create(a, i), 1.5f);
        wall_create(vec2_create(-i-1, a), 1.5f);
        wall_create(vec2_create(-a, -i-1), 1.5f);
    }
}

Wall* wall_create(vec2 position, f32 height)
{
    Wall* wall = malloc(sizeof(Wall));
    wall->position = position;
    wall->size = vec2_create(1, 1);
    wall->height = height;
    wall->top_tex = texture_get_id("wall_1");
    wall->side_tex = texture_get_id("wall_2");
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

