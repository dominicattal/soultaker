#include "internal.h"

extern GameContext game_context;

static i32 (*texture_getters[NUM_ENTITY_TYPES])(Entity* entity);
static void (*create_functions[NUM_ENTITY_TYPES])(Entity* entity);
static void (*update_functions[NUM_ENTITY_TYPES])(Entity* entity, f32 dt);
static void (*destroy_functions[NUM_ENTITY_TYPES])(Entity* entity);
static void (*state_setters[NUM_ENTITY_TYPES])(Entity* entity, i32 state);

void entity_init(void)
{
    entity_knight_init();
    texture_getters[ENTITY_KNIGHT] = entity_knight_get_texture;
    update_functions[ENTITY_KNIGHT] = entity_knight_update;
    create_functions[ENTITY_KNIGHT] = entity_knight_create;
    destroy_functions[ENTITY_KNIGHT] = entity_knight_destroy;
    state_setters[ENTITY_KNIGHT] = entity_knight_set_state;

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
    entity->state_timer = 0;
    entity->haste = 0;
    entity->speed = 3.5;
    entity->size = 1.0f;
    entity->health = 1;
    entity->flags = 0;
    entity->type = 0;
    entity->state = 0;
    entity_set_flag(entity, ENTITY_FLAG_UPDATE_FACING, 1);

    #ifdef DEBUG_BUILD
    assert(create_functions[entity->type] != NULL);
    #endif
    create_functions[entity->type](entity);

    list_append(game_context.entities, entity);
    return entity;
}

void entity_update(Entity* entity, f32 dt)
{
    entity->prev_position = entity->position;
    entity->position = vec3_add(entity->position, vec3_scale(entity->direction, entity->speed * dt));
    entity->state_timer += dt;
    if (entity_get_flag(entity, ENTITY_FLAG_UPDATE_FACING) && vec3_mag(entity->direction) > 0)
        entity->facing = vec2_create(entity->direction.x, entity->direction.z);

    #ifdef DEBUG_BUILD
    assert(update_functions[entity->type] != NULL);
    #endif
    update_functions[entity->type](entity, dt);
}

/*
entity->flags
00000000000000ddddddddccccccccba
a = ENTITY_FLAG_FRIENDLY
b = ENTITY_FLAG_UPDATE_FACING
c = state
d = type
*/

void entity_set_flag(Entity* entity, EntityFlagEnum flag, u32 val)
{
    entity->flags = (entity->flags & ~(1<<flag)) | (val<<flag);
}

bool entity_get_flag(Entity* entity, EntityFlagEnum flag)
{
    return (entity->flags >> flag) & 1;
}

void entity_set_state(Entity* entity, i32 state)
{
    state_setters[entity->type](entity, state);
}

i32 entity_get_direction(Entity* entity)
{
    f32 entity_rad = vec2_radians(entity->facing);
    f32 camera_rad = game_context.camera.yaw;
    return get_direction(entity_rad - camera_rad);
}

i32 entity_get_texture(Entity* entity)
{
    return texture_getters[entity->type](entity);
}

void entity_destroy(Entity* entity)
{
    #ifdef DEBUG_BUILD
    assert(destroy_functions[entity->type] != NULL);
    #endif
    destroy_functions[entity->type](entity);
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
