#ifndef API_H
#define API_H

#include "game/internal.h"
#include "util.h"

typedef struct {
    i32 (*entity_get_direction)(Entity*);
    vec3 (*game_get_nearest_player_position)(void);
    void (*game_set_player_position)(vec3);
    Map* (*map_load)(const char* path);
    void (*map_free)(Map* map);
    Entity* (*entity_create)(vec3, i32);
    i32 (*entity_map_id)(const char*);
    void (*tile_set_flag)(Tile*, TileFlagEnum, u32);
    bool (*tile_get_flag)(Tile*, TileFlagEnum);
    void (*tile_lava_collision)(Entity* entity);
    Projectile* (*projectile_create)(vec3);
    i32 (*texture_get_id)(const char*);
    vec3 (*vec3_create)(f32, f32, f32);
    vec2 (*vec2_create)(f32, f32);
    vec3 (*vec3_normalize)(vec3);
    Tile* (*tile_create)(vec2);
    Wall* (*wall_create)(vec2, f32);
    void* (*_st_malloc)(size_t, const char*, int);
    void (*_st_free)(void*, const char*, int);
    void (*_log_write)(LogLevel, const char*, const char*, int, ...);
} GlobalApi;

extern GlobalApi global_api;

#endif
