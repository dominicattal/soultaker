#include "../game.h"

extern GameContext game_context;

Obstacle* obstacle_create(vec2 position)
{
    Obstacle* obstacle = st_malloc(sizeof(Obstacle));
    obstacle->position = position;
    obstacle->size = 1.0f;
    return obstacle;
}

void obstacle_destroy(Obstacle* obstacle)
{
    st_free(obstacle);
}
