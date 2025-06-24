#ifndef ENTITY_KNIGHT_H
#define ENTITY_KNIGHT_H

#include "../../../src/api.h"
#include <math.h>

#define NUM_STATES 3

typedef struct {
    f32 shot_timer;
} Data;

static GlobalApi* api;

__declspec(dllexport)
void entity_enemy_init(GlobalApi* _api)
{
    api = _api;
}

__declspec(dllexport)
i32 entity_enemy_texture(Entity* entity)
{
    return 0;
}

__declspec(dllexport)
void entity_enemy_update(Entity* entity, f32 dt)
{
    vec3 position;
    vec2 direction;
    f32 dx, dz;
    Data* data = entity->data;
    data->shot_timer -= dt;
    if (data->shot_timer < 0) {
        position = api->game_get_nearest_player_position();
        dx = position.x - entity->position.x;
        dz = position.z - entity->position.z;
        direction = api->vec2_create(dx, dz);
        Projectile* proj = api->projectile_create(entity->position);
        proj->position.y = 0.5;
        proj->rotation = atan(dz / dx) + (dx > 0 ? PI : 0);
        entity->facing = direction;
        proj->direction = api->vec3_normalize(api->vec3_create(dx, 0.0, dz));
        proj->speed = 10;
        data->shot_timer = 0.5;
    }
}

__declspec(dllexport)
void entity_enemy_create(Entity* entity)
{
    Data* data = api->st_malloc(sizeof(Data));
    data->shot_timer = 0;
    entity->data = data;
}

__declspec(dllexport)
void entity_enemy_destroy(Entity* entity)
{
    api->st_free(entity->data);
}

__declspec(dllexport)
void entity_enemy_cleanup(void)
{
}

#endif
