#ifndef GAME_INTERNAL_H
#define GAME_INTERNAL_H

#include "../game.h"

typedef struct {
    f32 yaw, pitch, zoom, fov, move_speed, rotate_speed;
    f32 view[16], proj[16];
    vec3 position, facing, right, up;
    u32 matrices_ubo;
} Camera;

typedef struct {
    vec3 position;
    vec3 prev_position;
    vec3 direction;
    f32 speed;
    f32 hitbox_radius;
} Entity;

typedef struct {
    vec3 position;
    vec3 direction;
    f32 speed;
    f32 hitbox_radius;
} Project;

typedef struct {
    vec2 position;
    i32 tex;
} Tile;

typedef struct {
    vec2 position;
    f32 height;
    i32 top_tex, side_tex;
} Wall;

typedef struct {
    i32 tile_length, tile_capacity;
    GLfloat* tile_buffer;
    bool update_tile_buffer;
    i32 wall_length, wall_capacity;
    GLfloat* wall_buffer;
    bool update_wall_buffer;
    i32 entity_length, entity_capacity;
    GLfloat* entity_buffer;
    i32 proj_length, proj_capacity;
    GLfloat* proj_buffer;
} GameData;

typedef struct {
    GameData data;
    GameData data_swap;
    Entity* player;
    List* entities;
    List* tiles;
    List* walls;
    Camera camera;
    bool kill_thread;
    pthread_t thread_id;
    pthread_mutex_t data_mutex;
    f32 dt;
} GameContext;

extern GameContext game_context;

void entity_init(void);
Entity* entity_create(vec3 position);
void entity_update(Entity* entity, f32 dt);
void entity_destroy(Entity* entity);
void entity_cleanup(void);

void tile_init(void);
Tile* tile_create(vec2 position);
void tile_destroy(Tile* tile);
void tile_cleanup(void);

void wall_init(void);
Wall* wall_create(vec2 position, f32 height);
void wall_destroy(Wall* wall);
void wall_cleanup(void);

void camera_init(void);
void camera_update(void);
void camera_move(vec2 mag);
void camera_rotate(f32 mag, f32 dt);
void camera_tilt(f32 mag, f32 dt);
void camera_zoom(i32 mag);
void camera_cleanup(void);

vec3 camera_position(void);
vec3 camera_facing(void);
f32  camera_pitch(void);
f32  camera_yaw(void);

#endif
