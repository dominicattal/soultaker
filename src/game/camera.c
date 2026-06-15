#include "../game.h"
#include "../renderer.h"
#include "../window.h"
#include <math.h>

#define MIN_ZOOM 1
#define MAX_ZOOM 8
#define MIN_PITCH 0.3
#define MAX_PITCH 1.3

#define DEFAULT_YAW         PI
#define DEFAULT_PITCH       MAX_PITCH
#define DEFAULT_FOV         PI / 4
#define DEFAULT_ZOOM_LEVEL  4
#define DEFAULT_ROTSPEED    3.5
#define DEFAULT_TILTSPEED   3.5
#define DEFAULT_MOVESPEED   10
#define DEFAULT_POSITION    vec3_create(0, 5, 0)
#define Y_AXIS              vec3_create(0, 1, 0)

#define DISTANCE_FROM_TARGET 100

extern GameContext game_context;

static void update_orientation_vectors(Camera* camera)
{
    camera->facing.x = cos(camera->yaw) * cos(-camera->pitch);
    camera->facing.y = sin(-camera->pitch);
    camera->facing.z = sin(camera->yaw) * cos(-camera->pitch);
    camera->right = vec3_normalize(vec3_cross(Y_AXIS, camera->facing));
    camera->up = vec3_cross(camera->facing, camera->right);
}

static void lock_onto_target(Camera* camera)
{
    vec3 position;
    vec2 pos2;
    if (camera->follow && game_context.this_client->player.entity != NULL) {
        pos2 = game_context.this_client->player.entity->position;
        position = vec3_create(pos2.x, 0.0f, pos2.z);
        camera->position = vec3_sub(position, vec3_scale(camera->facing, DISTANCE_FROM_TARGET));
        camera->target = pos2;
    } else if (!camera->follow) {
        position = vec3_create(camera->target.x, 0, camera->target.y);
        camera->position = vec3_sub(position, vec3_scale(camera->facing, DISTANCE_FROM_TARGET));
    }
}

void camera_set_defaults(Camera* camera)
{
    camera->yaw = DEFAULT_YAW;
    camera->pitch = DEFAULT_PITCH;
    camera->zoom_level = DEFAULT_ZOOM_LEVEL;
    camera->zoom = window_height() / (16.0f * camera->zoom_level);
    camera->fov = DEFAULT_FOV;
    camera->minimap_zoom = 150;
    camera->move_speed = DEFAULT_MOVESPEED;
    camera->rotate_speed = DEFAULT_ROTSPEED;
    camera->tilt_speed = DEFAULT_TILTSPEED;
    camera->rotate_mag = 0;
    camera->tilt_mag = 0;
    camera->follow = true;
}

void camera_update(Camera* camera, f32 dt)
{
    camera_rotate(camera, dt);
    camera_tilt(camera, dt);
    update_orientation_vectors(camera);
    lock_onto_target(camera);
}

void camera_update_direction(i32 client_uid, vec2 mag)
{
    Client* client = game_context.uid_map[client_uid];
    Camera* camera = &client->camera;
    Player* player = &client->player;
    Entity* entity = player->entity;
    if (camera->follow && entity == NULL)
        return;
    vec2 direction = vec2_create(0, 0);
    vec2 facing, right;
    facing.x = camera->facing.x;
    facing.y = camera->facing.z;
    facing = vec2_normalize(facing);
    right.x = camera->right.x;
    right.y = camera->right.z;
    right = vec2_normalize(right);
    direction = vec2_add(direction, vec2_scale(facing, mag.x));
    direction = vec2_add(direction, vec2_scale(right, mag.z));
    direction = vec2_normalize(direction);
    if (camera->follow) {
        player->entity->direction = direction;
        if (vec2_mag(direction) != 0) {
            entity_set_flag(entity, ENTITY_FLAG_MOVING, true);
            if (!player_is_shooting())
                player->entity->facing = direction;
        } 
        else
            entity_set_flag(entity, ENTITY_FLAG_MOVING, false);
    } else {
        camera->target = vec2_add(camera->target, vec2_scale(direction, camera->move_speed * game_context.dt));
    }
}

void camera_update_rotation(i32 client_uid, f32 mag)
{
    Client* client = game_context.uid_map[client_uid];
    Camera* camera = &client->camera;
    camera->rotate_mag = mag;
}

void camera_update_tilt(i32 client_uid, f32 mag)
{
    Client* client = game_context.uid_map[client_uid];
    Camera* camera = &client->camera;
    camera->tilt_mag = mag;
}

void camera_rotate(Camera* camera, f32 dt)
{
    camera->yaw += camera->rotate_speed * camera->rotate_mag * dt;
    if (camera->yaw > 2 * PI)
        camera->yaw -= 2 * PI;
    else if (camera->yaw < 0)
        camera->yaw += 2 * PI;
}

void camera_tilt(Camera* camera, f32 dt)
{
    camera->pitch += camera->tilt_speed * camera->tilt_mag * dt;
    if (camera->pitch > MAX_PITCH - EPSILON)
        camera->pitch = MAX_PITCH - EPSILON;
    else if (camera->pitch < MIN_PITCH + EPSILON)
        camera->pitch = MIN_PITCH + EPSILON;
}

void camera_zoom(Camera* camera, i32 mag)
{
    camera->zoom_level += mag;
    if (camera->zoom_level < MIN_ZOOM)
        camera->zoom_level = MIN_ZOOM;
    else if (camera->zoom_level > MAX_ZOOM)
        camera->zoom_level = MAX_ZOOM;
    camera->zoom = window_height() / (16.0f * camera->zoom_level);
}

void camera_minimap_zoom(Camera* camera, i32 mag)
{
    camera->minimap_zoom += 5 * mag;
}

void camera_framebuffer_size_callback(void)
{
    if (game_context.this_client == NULL)
        return;
    Camera* camera = &game_context.this_client->camera;
    camera->zoom = window_height() / (16.0f * camera->zoom_level);
}

vec3 camera_get_position(void)
{
    if (game_context.this_client != NULL)
        return game_context.this_client->camera.position;
    return vec3_create(0,0,0);
}

vec2 camera_get_target_position(void)
{
    if (game_context.this_client != NULL)
        return game_context.this_client->camera.target;
    return vec2_create(0,0);
}

vec3 camera_get_facing(void)
{
    if (game_context.this_client != NULL)
        return game_context.this_client->camera.facing;
    return vec3_create(0,0,0);
}

f32 camera_get_pitch(void)
{
    if (game_context.this_client != NULL)
        return game_context.this_client->camera.pitch;
    return 0;
}

f32 camera_get_yaw(void)
{
    if (game_context.this_client != NULL)
        return game_context.this_client->camera.yaw;
    return 0;
}

f32 camera_get_zoom(void)
{
    if (game_context.this_client != NULL)
        return game_context.this_client->camera.zoom;
    return 0;
}

bool camera_toggle_lock(void)
{
    if (game_context.this_client != NULL)
        return game_context.this_client->camera.follow = !game_context.this_client->camera.follow;
    return false;
}

void camera_set_target(vec2 target)
{
    if (game_context.this_client != NULL)
        game_context.this_client->camera.target = target;
}
