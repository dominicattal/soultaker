#include "internal.h"

extern GameContext game_context;

void entity_init(void)
{
    game_context.entities = list_create();
}

Entity* entity_create(vec3 position)
{
    Entity* entity = malloc(sizeof(Entity));
    entity->position = position;
    return entity;
}

void entity_destroy(Entity* entity)
{
    free(entity);
}

void entity_cleanup(void)
{
    list_destroy(game_context.entities);
}
