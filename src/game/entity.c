#include "internal.h"

extern GameContext game_context;

void entity_init(void)
{
    game_context.entities = list_create();
    game_context.player = entity_create(vec3_create(0, 0, 0));
    entity_create(vec3_create(5, 0, 0));
    entity_create(vec3_create(0, 0, 4));
}

Entity* entity_create(vec3 position)
{
    Entity* entity = malloc(sizeof(Entity));
    entity->position = position;
    list_append(game_context.entities, entity);
    return entity;
}

void entity_destroy(Entity* entity)
{
    free(entity);
}

void entity_cleanup(void)
{
    if (game_context.entities != NULL)
        list_destroy(game_context.entities);
}
