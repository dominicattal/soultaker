#include "internal.h"

extern GameContext game_context;

void obstacle_init(void)
{
    game_context.obstacles = list_create();
    obstacle_create(vec2_create(5.0f, -5.0f));
}

Obstacle* obstacle_create(vec2 position)
{
    Obstacle* obstacle = malloc(sizeof(Obstacle));
    obstacle->position = position;
    list_append(game_context.obstacles, obstacle);
    return obstacle;
}

void obstacle_destroy(Obstacle* obstacle)
{
    free(obstacle);
}

void obstacle_cleanup(void)
{
    if (game_context.obstacles == NULL)
        return;
    for (i32 i = 0; i < game_context.obstacles->length; i++)
        free(list_get(game_context.obstacles, i));
    list_destroy(game_context.obstacles);
}
