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
    Entity* player;
    List* entities;
    Camera camera;
    bool kill_thread;
    pthread_t thread_id;
} GameContext;

extern GameContext game_context;

void entity_init(void);
Entity* entity_create(vec3 position);
void entity_destroy(Entity* entity);
void entity_cleanup(void);

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
