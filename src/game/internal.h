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
    Camera camera;
    bool kill_thread;
    pthread_t thread_id;
} GameContext;

extern GameContext game_context;

void camera_init(void);
void camera_move(vec3 mag, f32 dt);
void camera_rotate(f32 mag, f32 dt);
void camera_tilt(f32 mag, f32);
void camera_zoom(f32 mag, f32 dt);
void camera_cleanup(void);

vec3 camera_position(void);
vec3 camera_facing(void);
f32  camera_pitch(void);
f32  camera_yaw(void);

#endif
