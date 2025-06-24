#ifndef API_H
#define API_H

#include "game/internal.h"
#include "util.h"

typedef struct {
    i32 (*entity_get_direction)(Entity*);
    vec3 (*game_get_nearest_player_position)(void);
    Projectile* (*projectile_create)(vec3 position);
    vec3 (*vec3_create)(f32, f32, f32);
    vec2 (*vec2_create)(f32, f32);
    vec3 (*vec3_normalize)(vec3);
    void* (*_st_malloc)(size_t, const char*, int);
    void (*_st_free)(void*, const char*, int);
    void (*_log_write)(LogLevel, const char*, const char*, int, ...);
} GlobalApi;

extern GlobalApi global_api;

#endif
