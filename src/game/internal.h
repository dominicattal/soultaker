#ifndef GAME_INTERNAL_H
#define GAME_INTERNAL_H

#include "../game.h"
#include "../renderer.h"
#include "../util.h"

//**************************************************************************
// Forward Declarations
//**************************************************************************

typedef struct GameApi GameApi;
typedef struct Camera Camera;
typedef struct Weapon Weapon;
typedef struct Entity Entity;
typedef struct Player Player;
typedef struct Projectile Projectile;
typedef struct Tile Tile;
typedef struct Wall Wall;
typedef struct Barrier Barrier;
typedef struct Parstacle Parstacle;
typedef struct Obstacle Obstacle;
typedef struct Particle Particle;
typedef struct Parjicle Parjicle;

//**************************************************************************
// Camera definitions
//**************************************************************************

typedef struct Camera {
    f32 yaw, pitch, zoom, fov;
    f32 move_speed, rotate_speed, tilt_speed;
    f32 view[16], proj[16];
    vec3 position, facing, right, up;
    vec2 target;
    u32 matrices_ubo;
    u32 minimap_ubo;
    bool follow;
} Camera;

// These functions must be called on the render thread because
// they change OpenGL state

// Initialize and cleanup OpenGL buffers
void camera_init(void);
void camera_cleanup(void);

// Updates OpenGL buffers with current camera values.
void camera_update(void);

// It is only necessary to change the projection matrix
// when the screenspace is changed
void camera_framebuffer_size_callback(void);
void camera_zoom(i32 mag);

// Alter camera by magnitude specified. These values
// are taken as-is.
void camera_move(vec2 mag);
void camera_rotate(f32 mag);
void camera_tilt(f32 mag);

//**************************************************************************
// Maps. See docs/maps.md for more information
//**************************************************************************

typedef struct {
    const char* current_branch;
    const char* current_room_type;
    i32 num_rooms_left;
    i32 num_rooms_loaded;
    i32 male_x, male_z;
    bool no_path;
    bool create_no_path;
} LocalMapGenerationSettings;

void map_init(void);
void map_load(i32 id);
void map_cleanup(void);

// gets the tile or wall at (x, z), returns NULL if out of bounds
void* map_get(i32 x, i32 z);

// get the tile at (x, z) for the current map. returns
// NULL if it is out of bounds, the unit is empty, or is a wall
Tile* map_get_tile(i32 x, i32 z);

// get the wall at (x, z) for the current map. returns
// NULL if it is out of bounds, the unit is empty, or is a tile
Wall* map_get_wall(i32 x, i32 z);

// returns true if the tile at (x, z) is a wall
bool map_is_wall(i32 x, i32 z);

// returns whether coordinate in fog
bool map_fog_contains(vec2 position);
bool map_fog_contains_tile(Tile* tile);
bool map_fog_contains_wall(Wall* wall);

// defogs room that contains coordinate and adjacent rooms
void map_fog_explore(vec2 position);

// clears all the fog
void map_fog_clear(void);

Entity* room_create_entity(vec2 position, i32 id);
Wall* room_create_wall(vec2 position, f32 height, f32 width, f32 length);

//**************************************************************************
// Entity, Player definitions
//**************************************************************************

typedef struct Weapon {
    i32 id;
} Weapon;

typedef struct {
    f32 health, max_health;
    f32 mana, max_mana;
    f32 souls, max_souls;
    f32 speed;
} Stats;

typedef struct Entity {
    void* data;
    vec2 position;
    vec2 prev_position;
    vec2 direction;
    vec2 facing;
    f32 health, max_health;
    f32 speed;
    f32 elevation;
    f32 size;
    f32 hitbox_radius;
    f32 state_timer;
    f32 tile_timer;
    f32 frame_timer;
    f32 frame_speed;
    u32 flags;
    i32 id;
    i32 state;
    i32 frame;
} Entity;

typedef struct Player {
    Stats stats;
    Weapon weapon;
    Weapon swap_out;
    Entity* entity;
    vec2 position;
    f32 shot_timer;
    bool shooting;
    // store special states
    i32 state_idle;
    i32 state_walking;
    i32 state_shooting;
} Player;

typedef enum {
    ENTITY_FLAG_FRIENDLY,
    ENTITY_FLAG_IN_LAVA,
    ENTITY_FLAG_INVULNERABLE,
    ENTITY_FLAG_MOVING,
    ENTITY_FLAG_BOSS
} EntityFlagEnum;

// Initalize and cleanup entity info
// Loads entity data from config/entities.json
void entity_init(void);
void entity_cleanup(void);

// Destroy every entity
void entity_clear(void);

void entity_set_flag(Entity* entity, EntityFlagEnum flag, bool val);
bool entity_get_flag(Entity* entity, EntityFlagEnum flag);

i32 entity_get_id(const char* name);
i32 entity_get_state_id(Entity* entity, const char* name);
i32 entity_get_texture(Entity* entity);

void entity_set_direction(Entity* entity, vec2 direction);
void entity_set_facing(Entity* entity, vec2 facing);

// sets entity state and sets frame to 0. always use this
// rather than setting state directly.
void entity_set_state(Entity* entity, const char* name);

// Create, update, and destroy individual entities
// Each entitiy has a create, update, and delete
// function that are called when passed as arguments
// in these functions
Entity* entity_create(vec2 position, i32 id);
void entity_update(Entity* entity, f32 dt);
void entity_destroy(Entity* entity);

// this will change the damage taken by an entity depending
// on any modifiers it might have
void entity_damage(Entity* entity, f32 damage);

void entity_make_boss(Entity* entity);
void entity_boss_update(Entity* entity);
void entity_unmake_boss(Entity* entity);

// Assigns default entity for the player
void player_reset(void);
void player_update(Player* player, f32 dt);
void player_shoot(Player* player);
void player_swap_weapons(void);
bool player_is_shooting(void);

// Initalize and cleanup weapon info
// Loads weapon data from config/weapons.json
void weapon_init(void);
void weapon_cleanup(void);

i32 weapon_get_id(const char* name);
void weapon_shoot(Player* player, vec2 direction, vec2 target);

//**************************************************************************
// Tile definitions
//**************************************************************************

typedef void (*TileCollideFuncPtr)(GameApi* api, Entity* entity);

typedef struct Tile {
    TileCollideFuncPtr collide;
    vec2 position;
    i32 tex;
    u32 flags;
} Tile;

typedef enum {
    // Determine animation direction for tile
    // To get diagonal animation, set a horizontal
    // and vertical flag. Having both horizontal or
    // both vertical flags set will cancel out.
    TILE_FLAG_ANIMATE_HORIZONTAL_POS,
    TILE_FLAG_ANIMATE_HORIZONTAL_NEG,
    TILE_FLAG_ANIMATE_VERTICAL_POS,
    TILE_FLAG_ANIMATE_VERTICAL_NEG
} TileFlagEnum;
 
void tile_init(void);
void tile_clear(void);
void tile_set_flag(Tile* tile, TileFlagEnum flag, u32 val);
bool tile_get_flag(Tile* tile, TileFlagEnum flag);
Tile* tile_create(vec2 position);
void tile_destroy(Tile* tile);
void tile_cleanup(void);
void tile_lava_collision(Entity* entity);

//**************************************************************************
// Wall definitions
//**************************************************************************

typedef struct Wall {
    vec2 position;
    vec2 size;
    f32 height;
    i32 top_tex, side_tex;
} Wall;

void wall_init(void);
void wall_clear(void);
Wall* wall_create(vec2 position, f32 height);
void wall_destroy(Wall* wall);
void wall_cleanup(void);

// put all of the walls not in the tilemap in
// game_context.free_walls
void wall_update_free_walls(void);

//**************************************************************************
// Projectile definitions
//**************************************************************************

typedef void (*ProjectileUpdateFuncPtr)(Projectile*, f32);
typedef void (*ProjectileDestroyFuncPtr)(Projectile*);

typedef struct Projectile {
    vec2 position;
    vec2 direction;
    f32 elevation;
    f32 facing;
    f32 rotation;
    f32 speed;
    f32 size;
    f32 lifetime;
    u32 flags;
    i32 tex;
    ProjectileUpdateFuncPtr update;
    ProjectileDestroyFuncPtr destroy;
} Projectile;

typedef enum {
    PROJECTILE_FLAG_TEX_ROTATION,
    PROJECTILE_FLAG_FRIENDLY
} ProjectileFlagEnum;

// Initialize and cleanup projectiles
void projectile_init(void);
void projectile_cleanup(void);

// Destroy all projectiles
void projectile_clear(void);

// Projectiles call their update and destroy functions. They do
// not have a create function. They do not have ids mapped to a 
// function ptr (like entities) because it is not necessary to 
// know projectile information + it would be a headache.
Projectile* projectile_create(vec2 position);
void projectile_update(Projectile* projectile, f32 dt);
void projectile_destroy(Projectile* projectile);

void projectile_set_flag(Projectile* proj, ProjectileFlagEnum flag, bool val);
bool projectile_get_flag(Projectile* proj, ProjectileFlagEnum flag);

//**************************************************************************
// Obstacle definitions
// Obstacles have a circular hitbox
// They do not receive updates every frame
//**************************************************************************

typedef struct Obstacle {
    vec2 position;
    f32 size;
    i32 tex;
} Obstacle;

void obstacle_init(void);
void obstacle_clear(void);
Obstacle* obstacle_create(vec2 position);
void obstacle_destroy(Obstacle* obstacle);
void obstacle_cleanup(void);

//**************************************************************************
// Parstacle definitions
// Parstacles are obstacles without a hitbox (obstacle + particle)
// They do not receive updates every frame.
//**************************************************************************

typedef struct Parstacle {
    vec2 position;
    f32 size;
    i32 tex;
} Parstacle;

void parstacle_init(void);
void parstacle_clear(void);
Parstacle* parstacle_create(vec2 position);
void parstacle_destroy(Parstacle* parstacle);
void parstacle_cleanup(void);

//**************************************************************************
// Particle definitions
// Particles have no hitbox and are solid colors
//**************************************************************************

typedef struct Particle {
    vec3 position;
    vec3 direction;
    vec3 color;
    f32 lifetime;
    f32 speed;
    f32 size;
} Particle;

void particle_init(void);
void particle_clear(void);
Particle* particle_create(vec3 position);
void particle_update(Particle* particle, f32 dt);
void particle_destroy(Particle* particle);
void particle_cleanup(void);

//**************************************************************************
// Parjicle definitions
// Parjicles are particles with rotation (particle + projectile)
//**************************************************************************

typedef struct Parjicle {
    vec3 position;
    vec3 direction;
    vec3 color;
    f32 lifetime;
    f32 rotation;
    f32 speed;
    f32 size;
    u32 flags;
} Parjicle;

typedef enum {
    PARJICLE_FLAG_TEX_ROTATION
} ParjicleFlagEnum;

void parjicle_init(void);
void parjicle_clear(void);
Parjicle* parjicle_create(vec3 position);
void parjicle_update(Parjicle* parjicle, f32 dt);
void parjicle_set_flag(Parjicle* parjicle, ParjicleFlagEnum flag, u32 val);
bool parjicle_is_flag_set(Parjicle* parjicle, ParjicleFlagEnum flag);
void parjicle_destroy(Parjicle* parjicle);
void parjicle_cleanup(void);

//**************************************************************************
// Game Context
//**************************************************************************

// contains copy of values for thread-safety
// in getters
typedef struct {
    Stats player_stats;
    f32 boss_health;
    f32 boss_max_health;
    i32 num_bosses;
} GetterValues;

typedef struct {
    GetterValues values;
    Player player;
    List* entities;
    List* bosses;
    List* tiles;
    List* walls;
    List* free_walls;
    List* projectiles;
    List* parstacles;
    List* obstacles;
    List* particles;
    List* parjicles;
    Camera camera;
    pthread_t thread_id;
    pthread_mutex_t getter_mutex;
    sem_t game_loop_sem;
    f64 time;
    f32 dt;
    bool kill_thread;
    bool halt_input;
    bool halt_game_loop;
    bool halt_render;
    bool paused;
} GameContext;

// global game_context that everything on the game
// thread can access
extern GameContext game_context;

// setup and cleanup opengl buffers. this is
// done on the main thread on program creation
// and termination
void game_render_init(void);
void game_render_cleanup(void);

// change game's fbo when window is resized
void game_render_framebuffer_size_callback(void);

vec2 game_get_nearest_player_position(void);
void game_set_player_position(vec2 position);

// update game objects (entities, projectiles, etc)
void game_update(void);

// summon entity at player position or (0, 0) if player doesnt exist
void game_summon(i32 id);

// flag static objects for update
void game_render_update_obstacles(void);
void game_render_update_parstacles(void);
void game_render_update_tiles(void);
void game_render_update_walls(void);

void game_update_vertex_data(void);

//**************************************************************************
// API
//**************************************************************************

typedef struct GameApi {
    // Game
    vec2 (*game_get_nearest_player_position)(void);
    void (*game_set_player_position)(vec2);

    // Entity
    Entity* (*entity_create)(vec2, i32);
    void (*entity_make_boss)(Entity* entity);
    i32 (*entity_get_id)(const char*);
    i32 (*entity_get_state_id)(Entity*, const char*);
    void (*entity_set_flag)(Entity*, EntityFlagEnum, bool);
    void (*entity_set_state)(Entity*, const char*);

    // Wall
    Wall* (*wall_create)(vec2, f32);

    // Tile
    Tile* (*tile_create)(vec2);
    void (*tile_set_flag)(Tile*, TileFlagEnum, u32);
    bool (*tile_get_flag)(Tile*, TileFlagEnum);

    // Projectile
    Projectile* (*projectile_create)(vec2);
    void (*projectile_set_flag)(Projectile*, ProjectileFlagEnum, bool);

    // Map
    Entity* (*room_create_entity)(vec2, i32);
    Wall* (*room_create_wall)(vec2, f32, f32, f32);

    // Misc
    i32 (*texture_get_id)(const char*);

    // Util
    vec3 (*vec3_create)(f32, f32, f32);
    vec3 (*vec3_normalize)(vec3);
    vec3 (*vec3_sub)(vec3, vec3);
    vec2 (*vec2_create)(f32, f32);
    vec2 (*vec2_rotate)(vec2, f32);
    vec2 (*vec2_direction)(f32);
    vec2 (*vec2_sub)(vec2, vec2);
    vec2 (*vec2_normalize)(vec2);
    f32 (*vec2_radians)(vec2);

#ifdef DEBUG_BUILD
    void* (*_st_malloc)(size_t, const char*, int);
    void (*_st_free)(void*, const char*, int);
    void (*_log_write)(LogLevel, const char*, const char*, int, ...);
#else
    void* (*malloc)(size_t);
    void (*free)(void*);
    void (*_log_write)(LogLevel, const char*, ...);
#endif

} GameApi;

extern GameApi game_api;

#endif
