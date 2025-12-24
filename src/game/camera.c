#include "internal.h"
#include "../renderer.h"
#include "../window.h"
#include <math.h>

#define DEFAULT_YAW         PI
#define DEFAULT_PITCH       PI / 6
#define DEFAULT_FOV         PI / 4
#define DEFAULT_ZOOM        7
#define DEFAULT_ROTSPEED    3.5
#define DEFAULT_TILTSPEED   3.5
#define DEFAULT_MOVESPEED   3.5
#define DEFAULT_POSITION    vec3_create(0, 5, 0)
#define Y_AXIS              vec3_create(0, 1, 0)

#define DISTANCE_FROM_PLAYER 100

#define MIN_ZOOM 1
#define MAX_ZOOM 25
#define MIN_PITCH 0.3
#define MAX_PITCH 1.3

extern GameContext game_context;

static void update_orientation_vectors(void)
{
    game_context.camera.facing.x = cos(game_context.camera.yaw) * cos(-game_context.camera.pitch);
    game_context.camera.facing.y = sin(-game_context.camera.pitch);
    game_context.camera.facing.z = sin(game_context.camera.yaw) * cos(-game_context.camera.pitch);
    game_context.camera.right = vec3_normalize(vec3_cross(Y_AXIS, game_context.camera.facing));
    game_context.camera.up = vec3_cross(game_context.camera.facing, game_context.camera.right);
}
static void lock_onto_target(void)
{
    if (!game_context.camera.follow)
        return;
    if (game_context.player.entity == NULL)
        return;
    vec3 position;
    vec2 pos2 = game_context.player.entity->position;
    position = vec3_create(pos2.x, 0.0f, pos2.z);
    game_context.camera.position = vec3_sub(position, vec3_scale(game_context.camera.facing, DISTANCE_FROM_PLAYER));
    game_context.camera.target = pos2;
}

void camera_init(void)
{
    game_context.camera.yaw = DEFAULT_YAW;
    game_context.camera.pitch = DEFAULT_PITCH;
    game_context.camera.zoom = DEFAULT_ZOOM;
    game_context.camera.fov = DEFAULT_FOV;
    game_context.camera.move_speed = DEFAULT_MOVESPEED;
    game_context.camera.rotate_speed = DEFAULT_ROTSPEED;
    game_context.camera.tilt_speed = DEFAULT_TILTSPEED;
    game_context.camera.rotate_mag = 0;
    game_context.camera.tilt_mag = 0;
    game_context.camera.follow = true;
    update_orientation_vectors();
    lock_onto_target();
}

void camera_update(void)
{
    camera_rotate();
    camera_tilt();
    update_orientation_vectors();
    lock_onto_target();
}

void camera_update_direction(vec2 mag)
{
    Entity* entity = game_context.player.entity;
    if (game_context.camera.follow && entity == NULL)
        return;
    vec2 direction = vec2_create(0, 0);
    vec2 facing, right;
    facing.x = game_context.camera.facing.x;
    facing.y = game_context.camera.facing.z;
    facing = vec2_normalize(facing);
    right.x = game_context.camera.right.x;
    right.y = game_context.camera.right.z;
    right = vec2_normalize(right);
    direction = vec2_add(direction, vec2_scale(facing, mag.x));
    direction = vec2_add(direction, vec2_scale(right, mag.z));
    direction = vec2_normalize(direction);
    vec3 dir3 = vec3_create(direction.x, 0, direction.z);
    if (game_context.camera.follow) {
        game_context.player.entity->direction = direction;
        if (vec2_mag(direction) != 0) {
            entity_set_flag(entity, ENTITY_FLAG_MOVING, true);
            if (!player_is_shooting())
                game_context.player.entity->facing = direction;
        } 
        else
            entity_set_flag(entity, ENTITY_FLAG_MOVING, false);
    } else
        game_context.camera.position = vec3_add(game_context.camera.position, dir3);
}

void camera_update_rotation(f32 mag)
{
    game_context.camera.rotate_mag = mag;
}

void camera_update_tilt(f32 mag)
{
    game_context.camera.tilt_mag = mag;
}

void camera_rotate(void)
{
    game_context.camera.yaw += game_context.camera.rotate_speed * game_context.camera.rotate_mag * game_context.dt;
    if (game_context.camera.yaw > 2 * PI)
        game_context.camera.yaw -= 2 * PI;
    else if (game_context.camera.yaw < 0)
        game_context.camera.yaw += 2 * PI;
}

void camera_tilt(void)
{
    game_context.camera.pitch += game_context.camera.tilt_speed * game_context.camera.tilt_mag * game_context.dt;
    if (game_context.camera.pitch > MAX_PITCH - EPSILON)
        game_context.camera.pitch = MAX_PITCH - EPSILON;
    else if (game_context.camera.pitch < MIN_PITCH + EPSILON)
        game_context.camera.pitch = MIN_PITCH + EPSILON;
}

void camera_zoom(i32 mag)
{
    game_context.camera.zoom += mag;
    if (game_context.camera.zoom < MIN_ZOOM)
        game_context.camera.zoom = MIN_ZOOM;
    else if (game_context.camera.zoom > MAX_ZOOM)
        game_context.camera.zoom = MAX_ZOOM;
}

void camera_cleanup(void)
{
}

void camera_framebuffer_size_callback(void)
{
}

vec3 camera_get_position(void)
{
    return game_context.camera.position;
}

vec3 camera_get_facing(void)
{
    return game_context.camera.facing;
}

f32 camera_get_pitch(void)
{
    return game_context.camera.pitch;
}

f32 camera_get_yaw(void)
{
    return game_context.camera.yaw;
}

f32 camera_get_zoom(void)
{
    return game_context.camera.zoom;
}

