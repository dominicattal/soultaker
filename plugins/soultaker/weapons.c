#include "../../src/api.h"

st_export void weapon_pointer_attack(GameApi* api, Player* player, vec2 direction, vec2 target)
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

st_export void weapon_null_pointer_attack(GameApi* api, Player* player, vec2 direction, vec2 target)
{
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
