#include "api.h"

#define REGISTER(name) \
    .name = name

GlobalApi global_api = {
    // Game
    REGISTER(game_get_nearest_player_position),
    REGISTER(game_set_player_position),

    // Entity
    REGISTER(entity_create),
    REGISTER(entity_make_boss),
    REGISTER(entity_map_id),

    // Wall
    REGISTER(wall_create),

    // Tile
    REGISTER(tile_create),
    REGISTER(tile_set_flag),
    REGISTER(tile_get_flag),
    REGISTER(tile_lava_collision),

    // Map
    REGISTER(map_load),
    REGISTER(map_free),

    // Projectile
    REGISTER(projectile_create),

    // Misc
    REGISTER(texture_get_id),

    // Util
    REGISTER(vec3_create),
    REGISTER(vec3_normalize),
    REGISTER(vec2_create),
    REGISTER(vec2_rotate),
    REGISTER(_log_write),

#ifdef DEBUG_BUILD
    REGISTER(_st_malloc),
    REGISTER(_st_free),
#else
    REGISTER(malloc),
    REGISTER(free),
#endif

};
