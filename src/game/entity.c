#include "internal.h"

extern GameContext game_context;

static TextureEnum (*texture_getters[NUM_ENTITY_TYPES])(Entity* entity);

void entity_init(void)
{
    entity_knight_init();
    texture_getters[ENTITY_KNIGHT] = entity_knight_get_texture;

    #ifdef DEBUG_BUILD
    for (i32 i = 0; i < NUM_ENTITY_TYPES; i++)
        assert(texture_getters[i] != NULL);
    #endif

    game_context.entities = list_create();
    game_context.player.entity = entity_create(vec3_create(0, 0, 0));
    game_context.player.entity->direction = vec3_create(0, 0, 0);
    game_context.player.entity->size = 1.0;
    entity_set_flag(game_context.player.entity, ENTITY_FLAG_FRIENDLY, 1);
    entity_create(vec3_create(5, 0, 0));
    entity_create(vec3_create(0, 0, 4));
    entity_create(vec3_create(5, 0, 4));
}

Entity* entity_create(vec3 position)
{
    Entity* entity = malloc(sizeof(Entity));
    entity->position = position;
    entity->prev_position = position;
    entity->direction = vec3_create(0, 0, 0);
    entity->direction = vec3_normalize(entity->direction);
    entity->facing = vec2_create(1, 0);
    entity->speed = 3.5;
    entity->size = 1.0f;
    entity->health = 1;
    entity->flags = 0;
    list_append(game_context.entities, entity);
    return entity;
}

void entity_update(Entity* entity, f32 dt)
{
    entity->prev_position = entity->position;
    entity->position = vec3_add(entity->position, vec3_scale(entity->direction, entity->speed * dt));
}

void entity_set_friendly_flag(Entity* entity, u32 val)
{
    entity->flags = (entity->flags & ~1) | val;
}

void entity_set_flag(Entity* entity, EntityFlagEnum flag, u32 val)
{
    entity->flags = (entity->flags & ~(1<<flag)) | (val<<flag);
}

bool entity_is_flag_set(Entity* entity, EntityFlagEnum flag)
{
    return (entity->flags >> flag) & 1;
}

EntityType entity_get_type(Entity* entity)
{
    return (entity->flags >> 1) & ((1<<8)-1);
}

void entity_set_type(Entity* entity, EntityType type)
{
    entity->flags = (entity->flags & ~(((1<<8)-1)<<1)) | (type<<1);
}

i32 entity_get_direction(Entity* entity)
{
    f32 entity_rad = vec2_radians(entity->facing);
    f32 camera_rad = game_context.camera.yaw;
    return get_direction(entity_rad - camera_rad);
}

TextureEnum entity_get_tex(Entity* entity)
{
    return texture_getters[entity_get_type(entity)](entity);
}

i32 entity_get_state(Entity* entity)
{
    return (entity->flags >> 9) & ((1<<8)-1);
}

void entity_set_state(Entity* entity, i32 state)
{
    entity->flags = (entity->flags & ~(((1<<8)-1)<<9)) | (state<<9);
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
