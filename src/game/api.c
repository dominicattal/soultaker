#include "internal.h"

#define REGISTER(name) \
    .name = name

GameApi game_api = {
    // Game
    REGISTER(game_get_nearest_player_position),
    REGISTER(game_set_player_position),

    // Entity
    REGISTER(entity_create),
    REGISTER(entity_make_boss),
    REGISTER(entity_get_id),
    REGISTER(entity_get_state_id),
    REGISTER(entity_set_flag),
    REGISTER(entity_set_state),

    // Wall
    REGISTER(wall_create),

    // Tile
    REGISTER(tile_create),
    REGISTER(tile_set_flag),
    REGISTER(tile_get_flag),

    // Projectile
    REGISTER(projectile_create),
    REGISTER(projectile_set_flag),

    // Map
    REGISTER(room_create_entity),

    // Misc
    REGISTER(texture_get_id),

    // Util
    REGISTER(vec3_create),
    REGISTER(vec3_sub),
    REGISTER(vec3_normalize),
    REGISTER(vec2_create),
    REGISTER(vec2_rotate),
    REGISTER(vec2_direction),
    REGISTER(vec2_radians),
    REGISTER(vec2_sub),
    REGISTER(vec2_normalize),
    REGISTER(_log_write),

#ifdef DEBUG_BUILD
    REGISTER(_st_malloc),
    REGISTER(_st_free),
#else
    REGISTER(malloc),
    REGISTER(free),
#endif

};
