#include "api.h"

#define REGISTER(name) \
    .name = name

GlobalApi global_api = {
    REGISTER(entity_get_direction),
    REGISTER(game_get_nearest_player_position),
    REGISTER(projectile_create),
    REGISTER(entity_create),
    REGISTER(entity_map_id),
    REGISTER(texture_get_id),
    REGISTER(tile_create),
    REGISTER(wall_create),
    REGISTER(vec3_create),
    REGISTER(vec3_normalize),
    REGISTER(vec2_create),
    REGISTER(_st_malloc),
    REGISTER(_st_free),
    REGISTER(_log_write)
};
