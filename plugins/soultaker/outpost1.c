#include "../../src/api.h"
#include <string.h>

st_export void* outpost1_init(GameApi* api)
{
    return NULL;
}

st_export void outpost1_cleanup(GameApi* api, void* data)
{
}

st_export bool outpost1_generate(GameApi* api, LocalMapGenerationSettings* settings)
{
    if (strcmp(settings->current_room_type, "enemy") == 0) {
        if (settings->num_rooms_left == 1) {
            settings->create_no_path = true;
            return false;
        }
    }

    if (settings->num_rooms_left != 0)
        return false;

    if (strcmp(settings->current_branch, "main") == 0) {
        if (strcmp(settings->current_room_type, "spawn") == 0) {
            settings->current_room_type = "enemy";
            settings->num_rooms_left = 1;
            return false;
        }
    }

    return true;
}

st_export bool outpost1_branch(GameApi* api, LocalMapGenerationSettings* settings)
{
    return false;
    if (strcmp(settings->current_branch, "main") != 0)
        return false;
    if (rand() % 8 == 0) {
        settings->current_branch = "dead_end";
        settings->current_room_type = "enemy";
        settings->num_rooms_left = 5 + rand() % 3;
        return true;
    }
    return false;
}

#define IN_RANGE_THRESHOLD      10
#define OUT_OF_RANGE_THRESHOLD  20

typedef struct {
    vec2 spawn_point;
    f32 wander_cooldown;
    f32 wander_timer;
    f32 player_arc_rad;
} KnightData;

st_export void outpost1_knight_create(GameApi* api, Entity* entity)
{
    KnightData* data = api->st_malloc(sizeof(KnightData));
    data->spawn_point = entity->position;
    data->wander_cooldown = 0;
    data->player_arc_rad = 0;
    entity->health = entity->max_health = 10;
    entity->data = data;
    entity->state = api->entity_get_state_id(entity, "idle");
}

st_export void outpost1_knight_destroy(GameApi* api, Entity* entity)
{
    api->st_free(entity->data);
}

static bool outpost1_knight_player_in_range(GameApi* api, Entity* entity)
{
    return api->vec2_mag(api->vec2_sub(api->game_get_nearest_player_position(), entity->position)) < IN_RANGE_THRESHOLD;
}

static bool outpost1_knight_player_out_of_range(GameApi* api, Entity* entity)
{
    return api->vec2_mag(api->vec2_sub(api->game_get_nearest_player_position(), entity->position)) > OUT_OF_RANGE_THRESHOLD;
}

st_export void outpost1_knight_idle_update(GameApi* api, Entity* entity, f32 dt)
{
    vec2 offset, target, distance;
    KnightData* data = entity->data;
    if (outpost1_knight_player_in_range(api, entity)) {
        entity->state = api->entity_get_state_id(entity, "attack");
        api->log_write(DEBUG, "AA");
        return;
    }
    data->wander_cooldown -= dt;
    if (data->wander_cooldown < 0) {
        offset = api->vec2_create(api->randf() * 3, 0.0f);
        offset = api->vec2_rotate(offset, api->randf() * 2 * PI);
        target = api->vec2_add(data->spawn_point, offset);
        distance = api->vec2_sub(target, entity->position);
        data->wander_timer = api->vec2_mag(distance) / entity->speed;
        entity->direction = api->vec2_normalize(distance);
        entity->state = api->entity_get_state_id(entity, "wander");
    }
}

st_export void outpost1_knight_wander_update(GameApi* api, Entity* entity, f32 dt)
{
    KnightData* data = entity->data;
    if (outpost1_knight_player_in_range(api, entity)) {
        entity->state = api->entity_get_state_id(entity, "attack");
        api->log_write(DEBUG, "BB");
        return;
    }
    data->wander_timer -= dt;
    if (data->wander_timer > 0)
        return;
    entity->direction = api->vec2_create(0, 0);
    data->wander_cooldown = api->randf_range(3.0f, 7.0f);
    entity->state = api->entity_get_state_id(entity, "idle");
}

st_export void outpost1_knight_attack_update(GameApi* api, Entity* entity, f32 dt)
{
    KnightData* data = entity->data;
    vec2 direction, offset, target;
    vec2 player_position = api->game_get_nearest_player_position();
    f32 distance = api->vec2_mag(api->vec2_sub(player_position, entity->position));
    if (outpost1_knight_player_out_of_range(api, entity)) {
        entity->state = api->entity_get_state_id(entity, "idle");
        entity->direction = api->vec2_create(0, 0);
        return;
    }
    if (distance > 7) {
        direction = api->vec2_normalize(api->vec2_sub(player_position, entity->position));
        entity->direction = direction;
        data->player_arc_rad = api->vec2_radians(direction) + PI;
        return;
    }
    data->player_arc_rad = api->gmodf(data->player_arc_rad + dt, 2 * PI);
    offset = api->vec2_direction(data->player_arc_rad);
    offset = api->vec2_scale(offset, 3.0f);
    target = api->vec2_add(player_position, offset);
    entity->direction = api->vec2_normalize(api->vec2_sub(target, entity->position));
}

st_export void outpost1_big_room_create(GameApi* api)
{
    vec2 position = api->vec2_create(8, 8);
    i32 id = api->entity_get_id("outpost1_knight");
    api->room_create_entity(position, id);
    api->room_create_entity(position, id);
    api->room_create_entity(position, id);
    api->room_create_entity(position, id);
}
