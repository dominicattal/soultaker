#ifndef GAME_H
#define GAME_H

#include <stdlib.h>
#include "entity/entity.h"
#include "projectile/projectile.h"
#include "tile/tile.h"
#include "storage.h"

typedef struct {
    ProjectileStorage projectiles;
    EntityStorage entities;
    TileStorage tiles;
} Game;

extern Game game;

void game_init(void);
void game_setup(void);
void game_update(f32 dt);
void game_set_target(vec3f target);
void game_destroy(void);
void game_shoot(vec2f pos, f32 rotation, f32 tilt, f32 zoom, f32 ar);

#endif