#include "../../src/api.h"

static vec2 pointer_offsets[] = {
    { .x = -0.5, .z = 3.5 },
    { .x = -1, .z = 3 },
    { .x = -1.5, .z = 2.5 },
    { .x = -1, .z = 2.5 },
    { .x = -0.5, .z = 2.5 },
    { .x = -0.5, .z = 2 },
    { .x = -0.5, .z = 1.5 },
    { .x = -0.5, .z = 1 },
    { .x = -0.5, .z = 0.5 },
    { .x = -0.5, .z = 0 },
    { .x = 0, .z = 0 },
    { .x = 0.5, .z = 0 },
    { .x = 0.5, .z = 0.5 },
    { .x = 0.5, .z = 1 },
    { .x = 0.5, .z = 1.5 },
    { .x = 0.5, .z = 2 },
    { .x = 0.5, .z = 2.5 },
    { .x = 1, .z = 2.5 },
    { .x = 1.5, .z = 2.5 },
    { .x = 1.0, .z = 3 },
    { .x = 0.5, .z = 3.5 },
    { .x = 0, .z = 4 },
};

st_export void weapon_pointer_primary(GameApi* api, Player* player, vec2 direction, vec2 target)
{
    vec2 pos = player->entity->position;
    Projectile* proj = api->map_create_projectile(pos);
    proj->direction = direction;
    proj->size = 0.5;
    proj->speed = 20;
    proj->lifetime = 0.15;
    proj->facing = api->vec2_radians(direction);
    proj->tex = api->texture_get_id("bullet");
    api->projectile_set_flag(proj, PROJECTILE_FLAG_FRIENDLY, 1);
}

st_export void weapon_pointer_secondary(GameApi* api, Player* player, vec2 direction, vec2 target)
{
    vec2 player_pos = player->entity->position;
    for (size_t i = 0; i < sizeof(pointer_offsets) / sizeof(vec2); i++) {
        vec2 offset = pointer_offsets[i];
        offset.x -= 0.5;
        vec2 pos = api->vec2_add(player_pos, api->vec2_rotate(offset, api->vec2_radians(direction) - PI/2));
        Projectile* proj = api->map_create_projectile(pos);
        proj->direction = direction;
        proj->size = 0.5;
        proj->speed = 20;
        proj->lifetime = 0.15;
        proj->facing = api->vec2_radians(direction);
        proj->tex = api->texture_get_id("bullet");
        api->projectile_set_flag(proj, PROJECTILE_FLAG_FRIENDLY, 1);
    }
}

st_export void weapon_null_pointer_primary(GameApi* api, Player* player, vec2 direction, vec2 target)
{
    api->log_write(DEBUG, "test");
    vec2 pos = player->entity->position;
    Projectile* proj = api->map_create_projectile(pos);
    proj->direction = direction;
    proj->size = 0.5;
    proj->speed = 20;
    proj->lifetime = 0.15;
    proj->facing = api->vec2_radians(direction);
    proj->tex = api->texture_get_id("null_ptr");
    api->projectile_set_flag(proj, PROJECTILE_FLAG_FRIENDLY, 1);
}

st_export void weapon_null_pointer_secondary(GameApi* api, Player* player, vec2 direction, vec2 target)
{
    vec2 player_pos = player->entity->position;
    for (size_t i = 0; i < sizeof(pointer_offsets) / sizeof(vec2); i++) {
        vec2 offset = pointer_offsets[i];
        offset.x += 0.5;
        vec2 pos = api->vec2_add(player_pos, api->vec2_rotate(offset, api->vec2_radians(direction) - PI/2));
        Projectile* proj = api->map_create_projectile(pos);
        proj->direction = direction;
        proj->size = 0.5;
        proj->speed = 20;
        proj->lifetime = 0.15;
        proj->facing = api->vec2_radians(direction);
        proj->tex = api->texture_get_id("null_ptr");
        api->projectile_set_flag(proj, PROJECTILE_FLAG_FRIENDLY, 1);
    }
}
