#include "internal.h"
#include "../event.h"

#define REGISTER(name) \
    .name = name

GameApi game_api = {
    // Game
    REGISTER(game_get_nearest_player_position),
    REGISTER(game_set_player_position),

    // Entity
    REGISTER(entity_create),
    REGISTER(entity_get_flag),
    REGISTER(entity_get_id),
    REGISTER(entity_get_state_id),
    REGISTER(entity_set_flag),
    REGISTER(entity_set_state),

    // Wall
    REGISTER(wall_create),
    REGISTER(wall_set_flag),
    REGISTER(wall_get_flag),

    // Tile
    REGISTER(tile_create),
    REGISTER(tile_set_flag),
    REGISTER(tile_get_flag),

    // Projectile
    REGISTER(projectile_set_flag),

    // Trigger
    REGISTER(trigger_set_flag),
    REGISTER(trigger_get_flag),

    // Map
    REGISTER(map_get_data),
    REGISTER(map_set_interactable),
    REGISTER(map_orientation),
    REGISTER(map_create_projectile),
    REGISTER(map_create_trigger),
    REGISTER(map_make_boss),
    REGISTER(map_unmake_boss),
    REGISTER(room_create_entity),
    REGISTER(room_create_obstacle),
    REGISTER(room_create_parstacle),
    REGISTER(room_create_wall),
    REGISTER(room_create_trigger),
    REGISTER(room_set_tilemap_wall),
    REGISTER(room_set_tilemap_tile),
    REGISTER(room_position),

    // Misc
    REGISTER(texture_get_id),

    // Util
    REGISTER(vec3_create),
    REGISTER(vec3_sub),
    REGISTER(vec3_normalize),
    REGISTER(vec2_add),
    REGISTER(vec2_mag),
    REGISTER(vec2_create),
    REGISTER(vec2_scale),
    REGISTER(vec2_rotate),
    REGISTER(vec2_rotate180),
    REGISTER(vec2_direction),
    REGISTER(vec2_radians),
    REGISTER(vec2_sub),
    REGISTER(vec2_normalize),
    REGISTER(_log_write),
    REGISTER(randf),
    REGISTER(randf_range),
    REGISTER(gmodf),

    // event
    REGISTER(event_create_gui_create_notification),

#ifdef DEBUG_BUILD
    REGISTER(_st_malloc),
    REGISTER(_st_free),
#else
    REGISTER(malloc),
    REGISTER(free),
#endif

};
