#include "internal.h"
#include "../renderer.h"
#include "../window.h"
#include <math.h>

#define NEAR_CLIP_DISTANCE  0.001f
#define FAR_CLIP_DISTANCE   1000.0f
#define DEFAULT_YAW         PI
#define DEFAULT_PITCH       PI / 6
#define DEFAULT_FOV         PI / 4
#define DEFAULT_ZOOM        7
#define DEFAULT_ROTSPEED    3.5
#define DEFAULT_MOVESPEED   3.5
#define DEFAULT_POSITION    vec3_create(0, 5, 0)
#define Y_AXIS              vec3_create(0, 1, 0)

#define DISTANCE_FROM_PLAYER 100

#define MIN_ZOOM 1
#define MAX_ZOOM 15
#define MIN_PITCH 0
#define MAX_PITCH PI / 2

extern GameContext game_context;

static void view(f32 view[16], vec3 right, vec3 up, vec3 facing, vec3 position);
static void orthographic(f32 proj[16], f32 aspect_ratio, f32 zoom);
//static void perspective(f32 proj[16], f32 aspect_ratio, f32 fov);

static void update_orientation_vectors(void)
{
    game_context.camera.facing.x = cos(game_context.camera.yaw) * cos(-game_context.camera.pitch);
    game_context.camera.facing.y = sin(-game_context.camera.pitch);
    game_context.camera.facing.z = sin(game_context.camera.yaw) * cos(-game_context.camera.pitch);
    game_context.camera.right = vec3_normalize(vec3_cross(Y_AXIS, game_context.camera.facing));
    game_context.camera.up = vec3_cross(game_context.camera.facing, game_context.camera.right);
}

static void update_view_matrix(void)
{
    view(game_context.camera.view, game_context.camera.right, game_context.camera.up, game_context.camera.facing, game_context.camera.position);
    glBindBuffer(GL_UNIFORM_BUFFER, game_context.camera.matrices_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, 16 * sizeof(GLfloat), game_context.camera.view);
    glBufferSubData(GL_UNIFORM_BUFFER, 33 * sizeof(GLfloat), sizeof(GLfloat), &game_context.camera.pitch);
    glBufferSubData(GL_UNIFORM_BUFFER, 34 * sizeof(GLfloat), sizeof(GLfloat), &game_context.camera.yaw);
}

static void update_proj_matrix(void)
{
    orthographic(game_context.camera.proj, window_aspect_ratio(), game_context.camera.zoom);
    glBindBuffer(GL_UNIFORM_BUFFER, game_context.camera.matrices_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 16 * sizeof(GLfloat), 16 * sizeof(GLfloat), game_context.camera.proj);
    glBufferSubData(GL_UNIFORM_BUFFER, 32 * sizeof(GLfloat), sizeof(GLfloat), &game_context.camera.zoom);
}

static void lock_onto_target(void)
{
    if (!game_context.camera.follow)
        return;
    if (game_context.player.entity == NULL)
        return;
    vec3 position = game_context.player.entity->position;
    position.y = 0.0;
    game_context.camera.position = vec3_sub(position, vec3_scale(game_context.camera.facing, DISTANCE_FROM_PLAYER));
}

void camera_init(void)
{
    glGenBuffers(1, &game_context.camera.matrices_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, game_context.camera.matrices_ubo);
    glBufferData(GL_UNIFORM_BUFFER, 35 * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, UBO_INDEX_MATRICES, game_context.camera.matrices_ubo);

    game_context.camera.yaw = DEFAULT_YAW;
    game_context.camera.pitch = DEFAULT_PITCH;
    game_context.camera.zoom = DEFAULT_ZOOM;
    game_context.camera.fov = DEFAULT_FOV;
    game_context.camera.move_speed = DEFAULT_MOVESPEED;
    game_context.camera.rotate_speed = DEFAULT_ROTSPEED;
    game_context.camera.follow = true;
    update_orientation_vectors();
    lock_onto_target();
    update_view_matrix();
    update_proj_matrix();
}

void camera_reset(void)
{
    update_orientation_vectors();
    lock_onto_target();
    update_view_matrix();
    update_proj_matrix();
}

void camera_update(void)
{
    lock_onto_target();
    update_view_matrix();
}

void camera_move(vec2 mag, f32 dt)
{
    if (game_context.camera.follow && game_context.player.entity == NULL)
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
    direction = vec2_add(direction, vec2_scale(right, mag.y));
    direction = vec2_normalize(direction);
    vec3 dir3 = vec3_create(direction.x, 0, direction.y);
    if (game_context.camera.follow)
        game_context.player.entity->direction = dir3;
    else
        game_context.camera.position = vec3_add(game_context.camera.position, vec3_scale(dir3, dt));
}

void camera_rotate(f32 mag, f32 dt)
{
    game_context.camera.yaw += mag * dt * game_context.camera.rotate_speed;
    if (game_context.camera.yaw > 2 * PI)
        game_context.camera.yaw -= 2 * PI;
    else if (game_context.camera.yaw < 0)
        game_context.camera.yaw += 2 * PI;
    update_orientation_vectors();
    lock_onto_target();
    update_view_matrix();
}

void camera_tilt(f32 mag, f32 dt)
{
    game_context.camera.pitch += mag * dt * game_context.camera.rotate_speed;
    if (game_context.camera.pitch > MAX_PITCH - EPSILON)
        game_context.camera.pitch = MAX_PITCH - EPSILON;
    else if (game_context.camera.pitch < MIN_PITCH + EPSILON)
        game_context.camera.pitch = MIN_PITCH + EPSILON;
    update_orientation_vectors();
    lock_onto_target();
    update_view_matrix();
}

void camera_zoom(i32 mag)
{
    game_context.camera.zoom += mag;
    if (game_context.camera.zoom < MIN_ZOOM)
        game_context.camera.zoom = MIN_ZOOM;
    else if (game_context.camera.zoom > MAX_ZOOM)
        game_context.camera.zoom = MAX_ZOOM;
    update_proj_matrix();
}

void camera_cleanup(void)
{
    log_write(INFO, "Deleting camera buffers...");
    if (game_context.camera.matrices_ubo != 0)
        glDeleteBuffers(1, &game_context.camera.matrices_ubo);
    log_write(INFO, "Deleted camera buffers");
}

void camera_framebuffer_size_callback(void)
{
    update_proj_matrix();
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

/* ------------------------- */

static void view(f32 m[16], vec3 r, vec3 u, vec3 f, vec3 p)
{
    f32 k1 = p.x * r.x + p.y * r.y + p.z * r.z;
    f32 k2 = p.x * u.x + p.y * u.y + p.z * u.z;
    f32 k3 = p.x * f.x + p.y * f.y + p.z * f.z;
    m[0]  = r.x; m[1]  = u.x; m[2]  = f.x; m[3]  = 0.0f;
    m[4]  = r.y; m[5]  = u.y; m[6]  = f.y; m[7]  = 0.0f;
    m[8]  = r.z; m[9]  = u.z; m[10] = f.z; m[11] = 0.0f;
    m[12] = -k1; m[13] = -k2; m[14] = -k3; m[15] = 1.0f;
}

static void orthographic(f32 m[16], f32 ar, f32 zoom)
{
    f32 r, l, t, b, f, n;
    b = -(t = zoom);
    l = -(r = ar * zoom);
    f = FAR_CLIP_DISTANCE;
    n = NEAR_CLIP_DISTANCE;
    f32 val1, val2, val3, val4, val5, val6;
    val1 = 2 / (r - l);
    val2 = 2 / (t - b);
    val3 = 2 / (f - n);
    val4 = -(r + l) / (r - l);
    val5 = -(t + b) / (t - b);
    val6 = -(f + n) / (f - n);
    m[0]  = val1; m[1]  = 0.0f; m[2]  = 0.0f; m[3]  = 0.0f;
    m[4]  = 0.0f; m[5]  = val2; m[6]  = 0.0f; m[7]  = 0.0f;
    m[8]  = 0.0f; m[9]  = 0.0f; m[10] = val3; m[11] = 0.0f;
    m[12] = val4; m[13] = val5; m[14] = val6; m[15] = 1.0f;
}

/*
void perspective(f32 m[16], f32 ar, f32 fov)
{
    f32 a, b, c, d;
    f32 ncd = NEAR_CLIP_DISTANCE;
    f32 fcd = FAR_CLIP_DISTANCE;
    a = 1 / (ar * tan(fov / 2));
    b = 1 / (tan(fov / 2));
    c = (-ncd-fcd) / (ncd - fcd);
    d = (2 * fcd * ncd) / (ncd - fcd);
    m[0]  = a; m[1]  = 0; m[2]  = 0; m[3]  = 0;
    m[4]  = 0; m[5]  = b; m[6]  = 0; m[7]  = 0;
    m[8]  = 0; m[9]  = 0; m[10] = c; m[11] = 1;
    m[12] = 0; m[13] = 0; m[14] = d; m[15] = 0;
}
*/
