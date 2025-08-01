#ifndef API_H
#define API_H

#include "game/internal.h"
#include "util.h"

#define st_export __declspec(dllexport)

typedef struct {
    // Game
    vec2 (*game_get_nearest_player_position)(void);
    void (*game_set_player_position)(vec2);

    // Entity
    Entity* (*entity_create)(vec2, i32);
    void (*entity_make_boss)(Entity* entity);
    i32 (*entity_get_id)(const char*);
    i32 (*entity_get_state_id)(Entity*, const char*);
    void (*entity_set_flag)(Entity*, EntityFlagEnum, bool);
    void (*entity_set_state)(Entity*, const char*);

    // Wall
    Wall* (*wall_create)(vec2, f32);

    // Tile
    Tile* (*tile_create)(vec2);
    void (*tile_set_flag)(Tile*, TileFlagEnum, u32);
    bool (*tile_get_flag)(Tile*, TileFlagEnum);
    void (*tile_lava_collision)(Entity* entity);

    // Projectile
    Projectile* (*projectile_create)(vec2);
    void (*projectile_set_flag)(Projectile*, ProjectileFlagEnum, bool);

    // Misc
    i32 (*texture_get_id)(const char*);

    // Util
    vec3 (*vec3_create)(f32, f32, f32);
    vec3 (*vec3_normalize)(vec3);
    vec3 (*vec3_sub)(vec3, vec3);
    vec2 (*vec2_create)(f32, f32);
    vec2 (*vec2_rotate)(vec2, f32);
    vec2 (*vec2_direction)(f32);
    vec2 (*vec2_sub)(vec2, vec2);
    vec2 (*vec2_normalize)(vec2);
    f32 (*vec2_radians)(vec2);

#ifdef DEBUG_BUILD
    void* (*_st_malloc)(size_t, const char*, int);
    void (*_st_free)(void*, const char*, int);
    void (*_log_write)(LogLevel, const char*, const char*, int, ...);
#else
    void* (*malloc)(size_t);
    void (*free)(void*);
    void (*_log_write)(LogLevel, const char*, ...);
#endif

} GlobalApi;

extern GlobalApi global_api;

#endif
