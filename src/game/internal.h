#ifndef GAME_INTERNAL_H
#define GAME_INTERNAL_H

#include "entities/entities.h"
#include "../game.h"
#include "../renderer.h"

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
    vec2 facing;
    f32 speed;
    f32 size;
    f32 health;
    f32 haste;
    f32 state_timer;
    u32 flags;
    i32 type;
    i32 state;
    void* data;
} Entity;

typedef struct {
    vec3 position;
    vec3 direction;
    f32 rotation;
    f32 speed;
    f32 size;
    f32 lifetime;
    u32 flags;
} Projectile;

typedef struct {
    vec2 position;
    i32 tex;
} Tile;

typedef struct {
    vec2 position;
    vec2 size;
    f32 height;
    i32 top_tex, side_tex;
} Wall;

typedef struct {
    vec2 position;
    f32 size;
    i32 tex;
} Parstacle;

typedef struct {
    vec2 position;
    f32 size;
    i32 tex;
} Obstacle;

typedef struct {
    vec3 position;
    vec3 direction;
    vec3 color;
    f32 lifetime;
    f32 speed;
    f32 size;
} Particle;

typedef struct {
    vec3 position;
    vec3 direction;
    vec3 color;
    f32 lifetime;
    f32 rotation;
    f32 speed;
    f32 size;
    u32 flags;
} Parjicle;

typedef struct {
    Entity* entity;
    f32 shot_timer;
    bool shooting;
} Player;

typedef struct {
    i32 tile_length, tile_capacity;
    GLfloat* tile_buffer;
    bool update_tile_buffer;
    i32 wall_length, wall_capacity;
    GLfloat* wall_buffer;
    bool update_wall_buffer;
    i32 parstacle_length, parstacle_capacity;
    GLfloat* parstacle_buffer;
    bool update_parstacle_buffer;
    i32 obstacle_length, obstacle_capacity;
    GLfloat* obstacle_buffer;
    bool update_obstacle_buffer;
    i32 entity_length, entity_capacity;
    GLfloat* entity_buffer;
    i32 projectile_length, projectile_capacity;
    GLfloat* projectile_buffer;
    i32 particle_length, particle_capacity;
    GLfloat* particle_buffer;
    i32 parjicle_length, parjicle_capacity;
    GLfloat* parjicle_buffer;
} GameData;

typedef struct {
    GameData data;
    GameData data_swap;
    Player player;
    List* entities;
    List* tiles;
    List* walls;
    List* projectiles;
    List* parstacles;
    List* obstacles;
    List* particles;
    List* parjicles;
    Camera camera;
    bool kill_thread;
    pthread_t thread_id;
    pthread_mutex_t data_mutex;
    f32 dt;
} GameContext;

extern GameContext game_context;

//**************************************************************************

typedef enum {
    ENTITY_FLAG_FRIENDLY,
    ENTITY_FLAG_UPDATE_FACING
} EntityFlagEnum;

typedef enum {
    ENTITY_KNIGHT,
    NUM_ENTITY_TYPES
} EntityType;

typedef enum {
    PLAYER_STATE_IDLE,
    PLAYER_STATE_WALKING,
    PLAYER_STATE_SHOOTING,
    NUM_PLAYER_STATES
} PlayerStates;

// main api
void entity_init(void);
Entity* entity_create(vec3 position);
void entity_update(Entity* entity, f32 dt);
void entity_set_flag(Entity* entity, EntityFlagEnum flag, u32 val);
bool entity_get_flag(Entity* entity, EntityFlagEnum flag);
void entity_set_state(Entity* entity, i32 state);
i32 entity_get_direction(Entity* entity);
TextureEnum entity_get_texture(Entity* entity);
void entity_destroy(Entity* entity);
void entity_cleanup(void);

void player_update(Player* player, f32 dt);
void player_shoot(Player* player);
void player_set_state(Player* player, PlayerStates state);

// forward declarations
void entity_knight_init(void);
TextureEnum entity_knight_get_texture(Entity* entity);
void entity_knight_update(Entity* entity, f32 dt);
void entity_knight_create(Entity* entity);
void entity_knight_destroy(Entity* entity);
void entity_knight_set_state(Entity* entity, i32 state);

//**************************************************************************
 
void tile_init(void);
Tile* tile_create(vec2 position);
void tile_destroy(Tile* tile);
void tile_cleanup(void);

void wall_init(void);
Wall* wall_create(vec2 position, f32 height);
void wall_destroy(Wall* wall);
void wall_cleanup(void);

typedef enum {
    PROJECTILE_FLAG_TEX_ROTATION,
    PROJECTILE_FLAG_FRIENDLY
} ProjectileFlagEnum;

void projectile_init(void);
Projectile* projectile_create(vec3 position);
void projectile_update(Projectile* projectile, f32 dt);
void projectile_set_flag(Projectile* proj, ProjectileFlagEnum flag, u32 val);
bool projectile_get_flag(Projectile* proj, ProjectileFlagEnum flag);
void projectile_destroy(Projectile* projectile);
void projectile_cleanup(void);

void parstacle_init(void);
Parstacle* parstacle_create(vec2 position);
void parstacle_destroy(Parstacle* parstacle);
void parstacle_cleanup(void);

void obstacle_init(void);
Obstacle* obstacle_create(vec2 position);
void obstacle_destroy(Obstacle* obstacle);
void obstacle_cleanup(void);

void particle_init(void);
Particle* particle_create(vec3 position);
void particle_update(Particle* particle, f32 dt);
void particle_destroy(Particle* particle);
void particle_cleanup(void);

typedef enum {
    PARJICLE_FLAG_TEX_ROTATION
} ParjicleFlagEnum;

void parjicle_init(void);
Parjicle* parjicle_create(vec3 position);
void parjicle_update(Parjicle* parjicle, f32 dt);
void parjicle_set_flag(Parjicle* parjicle, ParjicleFlagEnum flag, u32 val);
bool parjicle_is_flag_set(Parjicle* parjicle, ParjicleFlagEnum flag);
void parjicle_destroy(Parjicle* parjicle);
void parjicle_cleanup(void);

void game_update(void);
void game_update_vertex_data(void);

void camera_init(void);
void camera_update(void);
void camera_move(vec2 mag);
void camera_rotate(f32 mag, f32 dt);
void camera_tilt(f32 mag, f32 dt);
void camera_zoom(i32 mag);
void camera_cleanup(void);
void camera_framebuffer_size_callback(void);

vec3 camera_get_position(void);
vec3 camera_get_facing(void);
f32  camera_get_pitch(void);
f32  camera_get_yaw(void);
f32  camera_get_pitch(void);
f32  camera_get_zoom(void);

#endif
