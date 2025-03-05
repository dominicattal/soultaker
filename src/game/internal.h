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
} Entity;

typedef struct {
    vec2 position;
} Tile;

typedef struct {
    GLint tile_length, tile_capacity;
    GLfloat* tile_buffer;
    GLint entity_length, entity_capacity;
    GLfloat* entity_buffer;
} GameData;

typedef struct {
    GameData data;
    GameData data_swap;
    Entity* player;
    List* entities;
    List* tiles;
    Camera camera;
    bool kill_thread;
    pthread_t thread_id;
    pthread_mutex_t data_mutex;
    f32 dt;
} GameContext;

extern GameContext game_context;

void entity_init(void);
Entity* entity_create(vec3 position);
void entity_destroy(Entity* entity);
void entity_cleanup(void);

void tile_init(void);
Tile* tile_create(vec2 position);
void tile_destroy(Tile* tile);
void tile_cleanup(void);

void camera_init(void);
void camera_move(vec2 mag, f32 dt);
void camera_rotate(f32 mag, f32 dt);
void camera_tilt(f32 mag, f32 dt);
void camera_zoom(i32 mag);
void camera_cleanup(void);

vec3 camera_position(void);
vec3 camera_facing(void);
f32  camera_pitch(void);
f32  camera_yaw(void);

#endif
