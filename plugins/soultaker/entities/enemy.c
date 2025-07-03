#ifndef ENTITY_KNIGHT_H
#define ENTITY_KNIGHT_H

#include "../../../src/api.h"
#include <math.h>

#define NUM_STATES 3

typedef struct {
    f32 shot_timer;
} Data;

st_export void entity_enemy_init(GlobalApi* api)
{
}

st_export void entity_enemy_update(GlobalApi* api, Entity* entity, f32 dt)
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
        proj->rotation = atan(dz / dx) + (dx > 0 ? 0 : PI);
        entity->facing = direction;
        proj->direction = api->vec3_normalize(api->vec3_create(dx, 0.0, dz));
        proj->speed = 10;
        proj->tex = api->texture_get_id("bullet");
        data->shot_timer = 0.5;
    }
}

st_export void entity_enemy_create(GlobalApi* api, Entity* entity)
{
    Data* data = api->st_malloc(sizeof(Data));
    data->shot_timer = 0;
    entity->data = data;
}

st_export void entity_enemy_destroy(GlobalApi* api, Entity* entity)
{
    api->st_free(entity->data);
}

st_export void entity_enemy_cleanup(GlobalApi* api)
{
}

#endif
