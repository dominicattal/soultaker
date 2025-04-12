#include "internal.h"

extern GameContext game_context;

void entity_init(void)
{
    game_context.entities = list_create();
    game_context.player.entity = entity_create(vec3_create(0, 0, 0));
    game_context.player.entity->direction = vec3_create(0, 0, 0);
    entity_create(vec3_create(5, 0, 0));
    entity_create(vec3_create(0, 0, 4));
    entity_create(vec3_create(5, 0, 4));
}

Entity* entity_create(vec3 position)
{
    Entity* entity = malloc(sizeof(Entity));
    entity->position = position;
    entity->prev_position = position;
    entity->direction = vec3_create(1, 0, 0.2);
    entity->direction = vec3_normalize(entity->direction);
    entity->speed = 3.5;
    entity->hitbox_radius = 0.5f;
    entity->health = 1;
    list_append(game_context.entities, entity);
    return entity;
}

void entity_update(Entity* entity, f32 dt)
{
    entity->prev_position = entity->position;
    entity->position = vec3_add(entity->position, vec3_scale(entity->direction, entity->speed * dt));
}

void entity_destroy(Entity* entity)
{
    free(entity);
}

void entity_cleanup(void)
{
    if (game_context.entities == NULL)
        return;
    for (i32 i = 0; i < game_context.entities->length; i++)
        free(list_get(game_context.entities, i));
    list_destroy(game_context.entities);
}
