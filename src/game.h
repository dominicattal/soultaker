#ifndef GAME_H
#define GAME_H

#include "util.h"
#include "window.h"

//**************************************************************************
// Constant(s) Declarations
//**************************************************************************

#define GRAVITY                 -9.8
#define PROJ_PIERCE_COOLDOWN    1
#define MAX_UID                 65535
#define MAP_MAX_WIDTH   1000
#define MAP_MAX_LENGTH  1000
#define PARTICLE_QUEUE_LENGTH 10000

typedef enum PacketEnum {

    PACKET_TEST,
    PACKET_HOST_UDP_PORT,
    PACKET_CLIENT_UDP_PORT,
    PACKET_HOST_TO_CLIENT_USERNAME,
    PACKET_HOST_TO_CLIENT_HOST_UID,
    PACKET_HOST_TO_CLIENT_CLIENT_UID,
    PACKET_CLIENT_TO_HOST_USERNAME,
    PACKET_MESSAGE,
    PACKET_LOAD_GAME,
    PACKET_PARTICLE,

    PACKET_CREATE_GAME_OBJ,
    PACKET_UPDATE_GAME_OBJ,
    PACKET_DESTROY_GAME_OBJ,

    PACKET_SYNC_CLIENT_ENTITY,
    PACKET_CREATE_MAP_NODES,
    PACKET_CLEAR_FOG,

    NUM_PACKET_TYPES
} PacketEnum;

typedef enum GameObj {
    GAME_OBJ_CLIENT,
    GAME_OBJ_ENTITY,
    GAME_OBJ_TILE,
    GAME_OBJ_WALL,
    GAME_OBJ_PROJECTILE,
    GAME_OBJ_OBSTACLE,
    GAME_OBJ_PARSTACLE,
    GAME_OBJ_PARTICLE,
    GMAE_OBJ_PARJICLE,
    GAME_OBJ_TRIGGER,
    GAME_OBJ_AOE,
    GAME_OBJ_LINE,
    NUM_GAME_OBJS,
    GAME_OBJ_NONE
} GameObj;

//**************************************************************************
// Forward Declarations
//**************************************************************************

typedef struct Line Line;
typedef struct Camera Camera;
typedef struct Weapon Weapon;
typedef struct Item Item;
typedef struct Synergy Synergy;
typedef struct Inventory Inventory;
typedef struct Entity Entity;
typedef struct Player Player;
typedef struct Projectile Projectile;
typedef struct AOE AOE;
typedef struct Tile Tile;
typedef struct Wall Wall;
typedef struct Barrier Barrier;
typedef struct Parstacle Parstacle;
typedef struct Obstacle Obstacle;
typedef struct Particle Particle;
typedef struct Parjicle Parjicle;
typedef struct Trigger Trigger;
typedef struct Map Map;
typedef struct MapNode MapNode;
typedef struct MapInfo MapInfo;
typedef struct LocalMapGenerationSettings LocalMapGenerationSettings;
typedef struct Connection Connection;
typedef struct Client Client;

typedef void (*TileCreateFuncPtr)(Tile*);
typedef void (*TileCollideFuncPtr)(Entity* entity);

typedef void (*TriggerEnterFunc)(Trigger*, Entity*);
typedef void (*TriggerStayFunc)(Trigger*, Entity*);
typedef void (*TriggerLeaveFunc)(Trigger*, Entity*);
typedef void (*TriggerDestroyFunc)(Trigger*);

typedef void (*InteractableFuncPtr)(void);

typedef void (*ProjectileUpdateFuncPtr)(Projectile*, f32);
typedef void (*ProjectileDestroyFuncPtr)(Projectile*);

// function called when room in generated. used to create game objects in that room
typedef void (*RoomCreateFuncPtr)(void);
// function called when player enters a room. i32 arg is the number of times that player entered
typedef void (*RoomEnterFuncPtr)(i32);
// function called when player exits a room.
typedef void (*RoomExitFuncPtr)(i32);
// create data for map that persists while map is in memory
// this data can be queried with map_get_data()
typedef void* (*RoomsetInitFuncPtr)(void);
// last function called before map is destroyed, used to cleanup allocations
// in RoomsetInitFuncPtr. void* arg is the data
typedef void (*RoomsetCleanupFuncPtr)(void*);
// true if at end of branch, false otherwise
typedef bool (*RoomsetGenerateFuncPtr)(LocalMapGenerationSettings*);
// true if should create branch, false otherwise
typedef bool (*RoomsetBranchFuncPtr)(LocalMapGenerationSettings*);

// used for items and synergies
typedef void (*UseFuncPtr)(Player*, vec2, vec2);

//**************************************************************************
// Camera declarations _camera
//**************************************************************************

typedef struct Camera {
    f64 yaw, pitch, zoom, fov, minimap_zoom;
    f32 move_speed, rotate_speed, tilt_speed;
    f32 rotate_mag, tilt_mag;
    vec3 position, facing, right, up;
    vec2 target;
    i32 zoom_level;
    bool follow;
} Camera;

void camera_update(Camera* camera, f32 dt);

// It is only necessary to change the projection matrix
// when the framebuffer is changed
void camera_framebuffer_size_callback(void);
void camera_zoom(Camera* camera, i32 mag);
void camera_minimap_zoom(Camera* camera, i32 mag);

void camera_update_direction(i32 client_uid, vec2 mag, f32 dt);
void camera_update_rotation(i32 client_uid, f32 mag);
void camera_update_tilt(i32 client_uid, f32 mag);
void camera_rotate(Camera* camera, f32 dt);
void camera_tilt(Camera* camera, f32 dt);

//**************************************************************************
// Particle _particle definitions
// Particles have no hitbox and are solid colors
//**************************************************************************

typedef void (*ParticleCreateFuncPtr)(Particle*);
typedef void (*ParticleUpdateFuncPtr)(Particle*, f32);
typedef void (*ParticleDestroyFuncPtr)(Particle*);

typedef struct Particle {
    void* data;
    vec3 position;
    vec3 velocity;
    vec3 acceleration;
    vec3 color;
    f32 lifetime;
    f32 size;
    i32 id;
} Particle;

void particle_init(void);
void particle_cleanup(void);

#define PARTICLE_CREATE(...) \
    (Particle) { \
        .data = NULL, \
        .position = vec3_create(0,0,0), \
        .velocity = vec3_create(0,0,0), \
        .acceleration = vec3_create(0,0,0), \
        .color = vec3_create(255,255,255), \
        .lifetime = 1.0, \
        .size = 0.1, \
        .id = -1, \
        __VA_ARGS__ \
    }

Particle* particle_create_from_struct(Particle particle);
//Particle* particle_create(vec3 position, vec3 velocity, vec3 acceleration, vec3 color, f32 lifetime, f32 size, i32 id);
void particle_update(Particle* particle, f32 dt);
void particle_destroy(Particle* particle);

i32 particle_get_id(const char* name);

//**************************************************************************
// Parjicle _parjicle definitions
// Parjicles are particles with rotation (particle + projectile)
//**************************************************************************

typedef void (*ParjicleUpdateFuncPtr)(Parjicle*, f32);
typedef void (*ParjicleDestroyFuncPtr)(Parjicle*);

typedef struct Parjicle {
    ParjicleUpdateFuncPtr update;
    ParjicleDestroyFuncPtr destroy;
    void* data;
    vec3 position;
    vec3 velocity;
    vec3 acceleration;
    vec3 color;
    f32 lifetime;
    f32 rotation;
    f32 size;
    i32 id;
    u32 flags;
} Parjicle;

typedef enum {
    PARJICLE_FLAG_TEX_ROTATION
} ParjicleFlagEnum;

Parjicle* parjicle_create(vec3 position);
void parjicle_update(Parjicle* parjicle, f32 dt);
void parjicle_set_flag(Parjicle* parjicle, ParjicleFlagEnum flag, u32 val);
bool parjicle_is_flag_set(Parjicle* parjicle, ParjicleFlagEnum flag);
void parjicle_destroy(Parjicle* parjicle);

//**************************************************************************
// Maps. _maps See docs/maps.md for more information
//**************************************************************************

typedef struct LocalMapGenerationSettings {
    const char* current_branch;
    const char* current_room_type;
    i32 num_rooms_left;
    i32 num_rooms_loaded;
    i32 male_x, male_z;
    bool no_path;
    bool create_no_path;
    bool succeed_even_if_no_path;
} LocalMapGenerationSettings;

typedef struct {
    union {
        i32 idx_x;
        i32 bl_bucket_idx;
    };
    union {
        i32 idx_z;
        i32 tr_bucket_idx;
    };
} IntPair;

typedef struct MapInfo {
    MapNode* spawn_node;
    MapNode* current_node;
    i32 tr_bucket_idx;
    i32 bl_bucket_idx;
} MapInfo;

typedef struct {
    const char* name;
    TileCreateFuncPtr create;
    TileCollideFuncPtr collide;
    u32 color;
    u32 top_tex;
    f32 height;
    union {
        u32 tex;
        u32 side_tex;
    };
    bool is_wall;
} TileColor;

typedef struct {
    TileColor* colors;
    i32 num_colors;
} Palette;

typedef struct {
    i32 u1, v1, u2, v2;
    i32 origin_u, origin_v;
    i32 loc_u, loc_v;
    TileColor* default_tile;
} Alternate;

typedef struct Room {
    i32 u1, v1, u2, v2;
    const char* name;
    const char* type;
    RoomCreateFuncPtr create;
    RoomEnterFuncPtr enter;
    RoomExitFuncPtr exit;
    List* male_alternates;
    List* female_alternates;
    bool rotate;
} Room;

typedef struct {
    i32 width, length;
    u32* pixels;
    Room* rooms;
    i32 num_rooms;
    void* data;
    Palette* palette;
    RoomsetInitFuncPtr init;
    RoomsetGenerateFuncPtr generate;
    RoomsetBranchFuncPtr branch;
    RoomsetCleanupFuncPtr cleanup;
} Roomset;

typedef enum {
    // Partitions map into equal-sized buckets 
    MAP_COLLIDE_SPATIAL_HASH,

    // Allocates no extra memory but is very slow
    MAP_COLLIDE_NAIVE
} MapCollisionStrategy;

typedef struct {
    List* entities;
    List* free_walls;
    List* projectiles;
    List* obstacles;
    List* triggers;
    List* aoes;
} Bucket;

typedef struct {
    i32 bucket_width;
    i32 num_buckets_wide; // x
    i32 num_buckets_long; // z
    i32 num_buckets;
    Bucket* buckets;
} SpatialHashData;

typedef struct {
    Particle buffer[PARTICLE_QUEUE_LENGTH+1];
    i32 head;
    i32 tail;
} ParticleQueue;

typedef struct Map {
    ParticleQueue particle_queue;
    i32 width, length;
    vec2 spawn_point;
    Roomset* roomset;
    MapNode* root;
    void** tilemap;
    MapNode** map_nodes;
    Quadmask* tile_mask;
    Quadmask* fog_mask;
    SpatialHashData spatial_hash_data;
    MapCollisionStrategy collision_strategy;
    List* bosses;
    List* entities;
    List* tiles;
    List* walls;
    List* free_walls;
    List* projectiles;
    List* obstacles;
    List* parstacles;
    List* particles;
    List* parjicles;
    List* triggers;
    List* aoes;
    List* lines;
    bool active;
    bool show_spatial_hash_lines;
} Map;

typedef struct MapNode {
    MapNode* parent;
    MapNode** children;
    Alternate* female_alternate;
    Room* room;
    List* male_alternates;
    i32 num_children;
    i32 origin_x, origin_z;
    i32 x1, x2, z1, z2;
    i32 orientation;
    i32 num_exits;
    i32 num_enters;
    bool visited;
    bool cleared;
} MapNode;

typedef struct {
    Quadmask* qm;
    Roomset* roomset;
} GlobalMapGenerationSettings;

void map_make_boss(char* name, Entity* entity);
void map_boss_update(Entity* entity);
void map_unmake_boss(Entity* entity);

void map_init(void);
i32  map_get_id(const char* name);
Map* map_create(i32 id);

void map_update(Map* map, f32 dt);
void map_set_active(Map* map);
void map_set_inactive(Map* map);
void map_destroy(Map* map);
void map_cleanup(void);

void map_destroy_projectiles_with_owner_id(i32 uid);

// switch collision strategy for map
void map_use_quadtree(Map* map, i32 split_threshold);
void map_use_spatial_hash(Map* map, i32 bucket_width);
void map_use_naive(Map* map);

void bucket_create(Bucket* bucket);
void bucket_destroy(Bucket* bucket);

void buckets_insert_trigger(Map* map, Trigger* trigger);
void buckets_insert_entity(Map* map, Entity* entity);
void buckets_insert_projectile(Map* map, Projectile* projectile);
void buckets_insert_free_wall(Map* map, Wall* wall);
void buckets_insert_obstacle(Map* map, Obstacle* obstacle);
void buckets_insert_aoe(Map* map, AOE* aoe);

void buckets_update_trigger(Map* map, Trigger* trigger);
void buckets_update_entity(Map* map, Entity* entity);
void buckets_update_projectile(Map* map, Projectile* projectile);
void buckets_update_free_wall(Map* map, Wall* wall);
void buckets_update_obstacle(Map* map, Obstacle* obstacle);
void buckets_update_aoe(Map* map, AOE* aoe);

void buckets_remove_trigger(Map* map, Trigger* trigger);
void buckets_remove_entity(Map* map, Entity* entity);
void buckets_remove_projectile(Map* map, Projectile* projectile);
void buckets_remove_free_wall(Map* map, Wall* wall);
void buckets_remove_obstacle(Map* map, Obstacle* obstacle);
void buckets_remove_aoe(Map* map, AOE* aoe);

// returns true if the tile at (x, z) is a wall
bool map_is_wall(Map* map, i32 x, i32 z);

// gets the tile or wall at (x, z), returns NULL if out of bounds
void* map_get(Map* map, i32 x, i32 z);

// get the tile at (x, z) for the current map. returns
// NULL if it is out of bounds, the unit is empty, or is a wall
Tile* map_get_tile(Map* map, i32 x, i32 z);

// get the wall at (x, z) for the current map. returns
// NULL if it is out of bounds, the unit is empty, or is a tile
Wall* map_get_wall(Map* map, i32 x, i32 z);

// replaces the tile or wall at (x, z) with tile or wall
void map_set_tile(Map* map, i32 x, i32 z, Tile* tile);
void map_set_wall(Map* map, i32 x, i32 z, Wall* wall);

// returns whether coordinate in fog
bool map_fog_contains(Map* map, vec2 position);
bool map_fog_contains_tile(Map* map, Tile* tile);
bool map_fog_contains_wall(Map* map, Wall* wall);

// defogs room that contains coordinate and adjacent rooms
void map_fog_explore(Map* map, vec2 position);

// clears all the fog
void map_fog_clear(Map* map);

void map_toggle_spatial_hash_lines(Map* map);

void map_handle_trigger_enter(Trigger* trigger, Entity* entity);
void map_handle_trigger_stay(Trigger* trigger, Entity* entity);
void map_handle_trigger_leave(Trigger* trigger, Entity* entity);

// gets the orientation of the current map node, where northwest is toward 0, 0 in uv map
// returns vec2(0,0) if no map node currently bounded
vec2 map_orientation(void);

void map_interactable_callback(InteractableFuncPtr fptr, Map* map, MapNode* map_node);
void map_set_interactable(const char* desc, InteractableFuncPtr func_ptr);

// get global data that is created on map generation
void* map_get_data(void);

vec2 room_to_map_position(vec2 position);
vec2 map_to_room_position(vec2 position);
vec3 room_to_map_position3(vec3 position);
vec3 map_to_room_position3(vec3 position);

void map_queue_particle(Particle particle);

// the following functions will return NULL if the map is not active
// a map is inactive when it is signaled to be destroyed. this is so that
// objects can free their memory without creating new memory

// create object in global map coords
Entity*         map_create_entity(vec2 position, i32 id);
Parjicle*       map_create_parjicle(vec3 position);
bool            map_create_particle(Particle particle);
Projectile*     map_create_projectile(vec2 position);
Trigger*        map_create_trigger(vec2 position, f32 radius);
AOE*            map_create_aoe(vec2 position, f32 lifetime);
Line*           map_create_line(void);

// create objects in local room coords
Entity*         room_create_entity(vec2 position, i32 id);
Obstacle*       room_create_obstacle(vec2 position);
Parstacle*      room_create_parstacle(vec2 position);
Projectile*     room_create_projectile(vec2 position);
Trigger*        room_create_trigger(vec2 position, f32 radius);
AOE*            room_create_aoe(vec2 position, f32 lifetime);
Wall*           room_create_wall(vec2 position, f32 height, f32 width, f32 length, u32 minimap_color);
Trigger*        room_create_trigger(vec2 position, f32 radius);
Parjicle*       room_create_parjicle(vec3 position);
bool            room_create_particle(Particle particle);
Line*           room_create_line(void);
Tile*           room_set_tilemap_tile(i32 x, i32 z, u32 minimap_color);
Wall*           room_set_tilemap_wall(i32 x, i32 z, f32 height, u32 minimap_color);

//**************************************************************************
// Line _line definitions
//**************************************************************************

typedef struct Line {
    vec3 pos1;
    vec3 color1;
    vec3 pos2;
    vec3 color2;
    f32 width;
    f32 lifetime;
    i32 uid;
    bool use_lifetime;
    bool is_spatial_hash_line;
} Line;

Line*   line_create(void);
void    line_update(Line* line, f32 dt);
void    line_destroy(Line* line);

//**************************************************************************
// Trigger _trigger definitions
//**************************************************************************

typedef struct Trigger {
    MapInfo map_info;
    TriggerEnterFunc enter;
    TriggerStayFunc stay;
    TriggerLeaveFunc leave;
    TriggerDestroyFunc destroy;
    void* data;
    Bitset* bitset;
    List* entities;
    vec2 position;
    f32 radius;
    u32 flags;
    i32 uid;
} Trigger;

typedef enum {
    TRIGGER_FLAG_DELETE,
    TRIGGER_FLAG_USED,
    TRIGGER_FLAG_ONCE,
    TRIGGER_FLAG_FRIENDLY,
    TRIGGER_FLAG_PLAYER,
} TriggerFlagEnum;

// create trigger at position with hitbox radius
// must set the enter, stay, leave, and destroy functions. they default to null
Trigger* trigger_create(vec2 position, f32 radius);
// if trigger->destroy is NULL, will call free on args
// otherwise, will run destroy function (without touching args)
void trigger_destroy(Trigger* trigger);
// checks entity list to find ones that left
void trigger_update(Trigger* trigger);
void trigger_set_flag(Trigger* trigger, TriggerFlagEnum flag, bool val);
bool trigger_get_flag(Trigger* trigger, TriggerFlagEnum flag);

//**************************************************************************
// Item _item definitions
//**************************************************************************

typedef enum StatEnum {
    STAT_HP,
    STAT_MAX_HP,
    STAT_HP_REGEN,
    STAT_MP,
    STAT_MAX_MP,
    STAT_MP_REGEN,
    STAT_SP,
    STAT_MAX_SP,
    STAT_SPEED,
    STAT_DAMAGE,
    NUM_STATS
} StatEnum;

typedef enum ItemTypeEnum {
    ITEM_WEAPON,
    ITEM_ARMOR,
    ITEM_ACCESSORY,
    ITEM_ABILITY,
    ITEM_MATERIAL,
    NUM_ITEM_TYPES
} ItemTypeEnum;

// not implemented
typedef enum ItemSubTypeEnum {
    ITEM_HELMET,
    ITEM_CHESTPLATE,
    ITEM_BOOTS,
    ITEM_SWORD,
    ITEM_STAFF,
    NUM_ITEM_SUBTYPES,
    ITEM_NO_SUBTYPE
} ItemSubTypeEnum;

typedef struct Item {
    ItemTypeEnum type;
    ItemSubTypeEnum subtype;
    f32 additive_stats[NUM_STATS];
    f32 multiplicative_stats[NUM_STATS];
    i32 id;
    f32 primary_cooldown;
    f32 primary_timer;
    f32 secondary_cooldown;
    f32 secondary_timer;
    bool equipped;
} Item;


// Initalize and cleanup weapon info
// Loads weapon data from config/weapons.json
void    item_init(void);
void    item_cleanup(void);

i32     item_get_id(const char* name);
i32     item_get_tex_id(i32 item_id);
Item*   item_create(i32 id);
void    item_update(Item* item, f32 dt);
char*   item_get_display_name(Item* item);
char*   item_get_tooltip(Item* item);
void    item_destroy(Item* item);

//**************************************************************************
// Synergy _synergy definitions
//**************************************************************************

typedef struct {
    i32* item_ids;
    i32 num_items;
    char* name;
    char* tooltip;
    char* display_name;
    UseFuncPtr primary;
    UseFuncPtr secondary;
    UseFuncPtr cast;
    UseFuncPtr use;
} SynergyInfo;

typedef struct {
    SynergyInfo* infos;
    i32 num_synergies;
    const char* current_synergy;
} SynergyContext;

typedef struct Synergy {
    f32 primary_cooldown;
    f32 primary_timer;
    f32 secondary_cooldown;
    f32 secondary_timer;
    f32 cast_cooldown;
    f32 cast_timer;
    f32 use_cooldown;
    f32 use_timer;
    i32 id;
} Synergy;

extern SynergyContext synergy_context;

void    synergy_init(void);
void    synergy_cleanup(void);
Synergy* synergy_create(i32 id);
void    synergy_update(Synergy* synergy, f32 dt);
i32     synergy_get_id(const char* name);
char*   synergy_get_name(i32 id);

//**************************************************************************
// Inventory _inventory definitions
//**************************************************************************

typedef struct Inventory {
    Item** items;
    Item** armor_slots[3];
    Item*** weapon_slots;
    Item*** ability_slots;
    Item*** misc_slots;
    Synergy** synergies;

    i32 num_items;
    i32 num_armor_slots;
    i32 num_weapon_slots;
    i32 num_ability_slots;
    i32 num_misc_slots;
    i32 num_synergies;
} Inventory;

void    inventory_refresh(void);
void    inventory_swap_items(Item** slot1, Item** slot2);
void    inventory_move_item(Item** slot);
void    inventory_shoot_weapons_primary(Player* player, vec2 direction, vec2 target);
void    inventory_shoot_weapons_secondary(Player* player, vec2 direction, vec2 target);
void    inventory_cast_abilities(Player* player, vec2 direction, vec2 target);

//**************************************************************************
// Entity, Player _entity _player definitions
//**************************************************************************

typedef void (*EntityCreateFuncPtr)(Entity*);
typedef void (*EntityDestroyFuncPtr)(Entity*);
typedef void (*EntityUpdateFuncPtr)(Entity*, f32);

typedef struct Stats {
    f32 health, max_health;
    f32 mana, max_mana;
    f32 souls, max_souls;
    f32 speed;
} Stats;

typedef struct Entity {
    MapInfo map_info;
    void* data;
    Player* player;
    vec2 position;
    vec2 prev_position;
    vec2 direction;
    vec2 facing;
    f32 health, max_health;
    f32 speed;
    f32 armor;
    f32 magic_resistance;
    f32 elevation;
    f32 size;
    f32 hitbox_radius;
    f32 state_timer;
    f32 tile_timer;
    f32 frame_timer;
    f32 frame_speed;
    u32 flags;
    i32 state;
    i32 frame;
    // id = type, uid = unique identifier, should rename these at some point
    i32 id;
    i32 uid;
} Entity;

typedef struct Player {
    Inventory inventory;
    f32 base_stats[NUM_STATS];
    f32 stats[NUM_STATS];
    Entity* entity;
    vec2 position;

    i32 entity_uid;
    bool synced;

    // store special states
    i32 state_idle;
    i32 state_walking;
    i32 state_shooting;

    bool shooting_primary;
    bool shooting_secondary;
} Player;

typedef enum {
    ENTITY_FLAG_AUTO_FREE_DATA,
    ENTITY_FLAG_PLAYER,
    ENTITY_FLAG_FRIENDLY,
    ENTITY_FLAG_IN_LAVA,
    ENTITY_FLAG_INVULNERABLE,
    ENTITY_FLAG_MOVING,
    ENTITY_FLAG_BOSS,
    ENTITY_FLAG_HIT_WALL
} EntityFlagEnum;

// Initalize and cleanup entity info
// Loads entity data from config/entities.json
void entity_init(void);
void entity_cleanup(void);

// functions for interacting with entity in binary format
size_t  entity_sizeof(void);
char*   entity_write(Entity* entity, char* buffer);
Entity* entity_read(char* buffer);

// Destroy every entity
void entity_clear(void);

void entity_set_flag(Entity* entity, EntityFlagEnum flag, bool val);
bool entity_get_flag(Entity* entity, EntityFlagEnum flag);

i32 entity_get_id(const char* name);
i32 entity_get_state_id(Entity* entity, const char* name);
i32 entity_get_texture(Entity* entity);

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

i32 entity_get_id(const char* name);

// Assigns default entity for the player
void player_reset(i32 client_uid, Entity* entity);
void player_update(Player* player, f32 dt);
void player_shoot_primary(Player* player);
void player_shoot_secondary(Player* player);
void player_cast(Player* player);
void player_swap_weapons(void);
bool player_is_shooting(void);
bool player_is_casting(void);
void player_cleanup(Player* player);

//**************************************************************************
// Tile %tile definitions
//**************************************************************************

typedef struct Tile {
    TileCollideFuncPtr collide;
    vec2 position;
    i32 tex;
    u32 minimap_color;
    u32 flags;
    i32 uid;
} Tile;

typedef enum {
    // Determine animation direction for tile
    // To get diagonal animation, set a horizontal
    // and vertical flag. Having both horizontal or
    // both vertical flags set will cancel out.
    TILE_FLAG_ANIMATE_HORIZONTAL_POS,
    TILE_FLAG_ANIMATE_HORIZONTAL_NEG,
    TILE_FLAG_ANIMATE_VERTICAL_POS,
    TILE_FLAG_ANIMATE_VERTICAL_NEG,

    // rather than remove tile from list, flag as active
    // to prevent O(n) list updates
    TILE_FLAG_ACTIVE
} TileFlagEnum;
 
size_t  tile_sizeof(void);
char*   tile_write(Tile* tile, char* buffer);
Tile*   tile_read(char* buffer);
void    tile_set_flag(Tile* tile, TileFlagEnum flag, bool val);
bool    tile_get_flag(Tile* tile, TileFlagEnum flag);
Tile*   tile_create(vec2 position, u32 minimap_color);
void    tile_destroy(Tile* tile);
void    tile_lava_collision(Entity* entity);

//**************************************************************************
// Wall _wall definitions
//**************************************************************************

typedef struct Wall {
    MapInfo map_info;
    vec2 position;
    vec2 size;
    f32 height;
    i32 top_tex, side_tex;
    u32 minimap_color;
    u32 flags;
    i32 uid;
} Wall;

typedef enum {
    WALL_FLAG_ACTIVE
} WallFlagEnum;

size_t  wall_sizeof(void);
char*   wall_write(Wall* wall, char* buffer);
Wall*   wall_read(char* buffer);
Wall*   wall_create(vec2 position, f32 height, u32 minimap_color);
void    wall_set_flag(Wall* wall, WallFlagEnum flag, bool val);
bool    wall_get_flag(Wall* wall, WallFlagEnum flag);
void    wall_destroy(Wall* wall);

//**************************************************************************
// Projectile _projectile definitions
//**************************************************************************

typedef struct Projectile {
    MapInfo map_info;
    ProjectileUpdateFuncPtr update;
    ProjectileDestroyFuncPtr destroy;
    void* data;
    vec2 position;
    vec2 direction;
    f32 elevation;
    f32 facing;
    f32 rotation;
    f64 speed;
    f32 size;
    f32 prev_size;
    f32 lifetime;
    f32 pierce_timer;
    u32 flags;
    i32 owner_uid;
    i32 tex;
    i32 uid;
} Projectile;

typedef enum {
    PROJECTILE_FLAG_AUTO_FREE_DATA,
    PROJECTILE_FLAG_TEX_ROTATION,
    PROJECTILE_FLAG_PIERCE,
    PROJECTILE_FLAG_IGNORE_LIFETIME,
    PROJECTILE_FLAG_FRIENDLY
} ProjectileFlagEnum;

// Projectiles call their update and destroy functions. They do
// not have a create function. They do not have ids mapped to a 
// function ptr (like entities) because it is not necessary to 
// know projectile information + it would be a headache.
Projectile* projectile_create(vec2 position);
void projectile_update(Projectile* projectile, f32 dt);
void projectile_destroy(Projectile* projectile);

void projectile_set_flag(Projectile* proj, ProjectileFlagEnum flag, bool val);
bool projectile_get_flag(Projectile* proj, ProjectileFlagEnum flag);

size_t      projectile_sizeof(void);
Projectile* projectile_read(char* buffer);
void        projectile_write(Projectile* projectile, char* buffer);

//**************************************************************************
// AOE _aoe definitions
//**************************************************************************

typedef void (*AOEUpdateFuncPtr)(AOE*, f32);
typedef void (*AOEDestroyFuncPtr)(AOE*);

typedef struct AOE {
    MapInfo map_info;
    AOEUpdateFuncPtr update;
    AOEDestroyFuncPtr destroy;
    void* data;
    vec2 position;
    vec2 prev_position;
    f32 radius;
    f32 prev_radius;
    f32 damage;
    f32 lifetime;
    f32 timer;
    f32 cooldown;
    u32 flags;
} AOE;

typedef enum {
    AOE_FLAG_FRIENDLY,
    AOE_FLAG_LINGER,
    AOE_FLAG_USED
} AOEFlagEnum;

// AOEs call their update and destroy functions. They do
// not have a create function. They do not have ids mapped to a 
// function ptr (like entities) because it is not necessary to 
// know aoe information + it would be a headache.
AOE* aoe_create(vec2 position, f32 lifetime);
void aoe_update(AOE* aoe, f32 dt);
void aoe_destroy(AOE* aoe);

void aoe_set_flag(AOE* proj, AOEFlagEnum flag, bool val);
bool aoe_get_flag(AOE* proj, AOEFlagEnum flag);

//**************************************************************************
// Obstacle _obstacle definitions
// Obstacles have a circular hitbox
// They do not receive updates every frame
//**************************************************************************

typedef struct Obstacle {
    MapInfo map_info;
    vec2 position;
    f32 size;
    i32 tex;
    i32 uid;
} Obstacle;

Obstacle* obstacle_create(vec2 position);
void obstacle_destroy(Obstacle* obstacle);

size_t      obstacle_sizeof(void);
Obstacle*   obstacle_read(char* buffer);
void        obstacle_write(Obstacle* obstacle, char* buffer);

//**************************************************************************
// Parstacle _parstacle definitions
// Parstacles are obstacles without a hitbox (obstacle + particle)
// They do not receive updates every frame.
//**************************************************************************

typedef struct Parstacle {
    vec2 position;
    f32 size;
    i32 tex;
    i32 uid;
} Parstacle;

Parstacle* parstacle_create(vec2 position);
void parstacle_destroy(Parstacle* parstacle);

size_t parstacle_sizeof(void);
Parstacle* parstacle_read(char* buffer);
void parstacle_write(Parstacle* parstacle, char* buffer);

//**************************************************************************
// Client _client
//**************************************************************************

typedef struct Client {
    Camera camera;
    Player player;

    // socket to communicate with client. bounding to other machine
    Socket* tcp_socket;

    // socket for sendto and recvfrom. bounding to this machine
    Socket* udp_socket;

    // socket address for this client
    SocketAddr* udp_address;

    char* username;
    i32 uid;
} Client;

Client* client_create(void);
void client_update(Client* client, f32 dt);
void client_destroy(Client* client);

// client owns username
void client_set_username(Client* client, char* username);

// functions for mp clients grouped together
void client_change_map(void);
void client_sync_entity(Packet* packet);
void client_map_clear_fog(Packet* packet);
void client_map_create_map_nodes(Packet* packet);
Map* client_map_create(void);
void client_map_update(Map* map, f32 dt);
void client_map_create_game_object(Packet* packet);
void client_map_update_game_object(Packet* packet);
void client_map_destroy_game_object(Packet* packet);
void client_map_create_particle(Packet* packet);

//**************************************************************************
// Game Context
//**************************************************************************

typedef struct {

    bool glfw_key_down[512];

    void* uid_map[MAX_UID]; 
    GameObj uid_map_type[MAX_UID];
    i32 uid_cursor;

    NetContext* net;
    char* host_ip;
    char* host_tcp_port;
    char* host_udp_port;
    List* clients;
    Client* this_client;
    Client* host_client;

    List_i32* created_uids;
    List_i32* freed_uids;

    pthread_t net_tcp_listen_thread_id;
    pthread_t net_udp_listen_thread_id;
    Map* current_map;
    pthread_t thread_id;
    pthread_mutex_t handler_thread_mutex;
    pthread_mutex_t getter_mutex;
    f64 time;
    f32 timestep;
    f32 net_timer;
    i32 tps;
    f32 real_dt;
    bool kill_thread;
    bool halt_input;
    bool halt_render;
    bool paused;
    bool hosting;
    bool singleplayer;
    bool cursor_moved;

} GameContext;

// global game_context that everything on the game
// thread can access
extern GameContext game_context;

// map an object to a uid and return assigned uid. 
// returns assigned uid and puts obj in game_context.uid_map
// returns -1 and doesnt do anything if each uid is in use
i32  game_map_uid(void* obj, GameObj type);
void game_set_uid(void* obj, GameObj type, i32 uid);
void game_free_uid(i32 uid);

// manage networking
void game_net_start_hosting(const char* ip, const char* port);
void game_net_stop_hosting(void);
void game_net_join(const char* ip, const char* port);
void game_net_cleanup(void);
void game_net_set_host_ip(const char* ip);
void game_net_set_host_tcp_port(const char* port);
void game_net_set_host_udp_port(const char* port);

void game_net_send_tcp_packet_to_clients(Packet* packet);
void game_net_send_udp_packet_to_clients(Packet* packet);
void game_net_send_packet_udp(Client* client, Packet* packet);
void game_net_send_packet_tcp(Client* client, Packet* packet);

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

void game_change_map(i32 id);

size_t game_object_write(GameObj type, void* obj, char* buffer);

//**************************************************************************
// Collision functions
//**************************************************************************

void collide_entity_wall(Entity* entity, Wall* wall);
void collide_entity_tile(Entity* entity, Tile* tile);
void collide_entity_obstacle(Entity* entity, Obstacle* obstacle);
void collide_entity_projectile(Entity* entity, Projectile* projectile);
void collide_entity_trigger(Entity* entity, Trigger* trigger);
void collide_entity_aoe(Entity* entity, AOE* aoe);
void collide_projectile_wall(Projectile* projectile, Wall* wall);
void collide_projectile_obstacle(Projectile* projectile, Obstacle* obstacle);

//**************************************************************************
// Declarations
//**************************************************************************

f32 game_get_boss_health(void);
f32 game_get_boss_max_health(void);

void game_halt_input(void);
void game_resume_input(void);
void game_halt_render(void);
void game_resume_render(void);
void game_pause(void);
void game_resume(void);

void game_init(void);
void game_cleanup(void);
void game_process_input(f32 dt);
void game_update_keys(void);
void game_render(void);
void game_load_starting_area(void);

char* weapon_get_name(i32 id);
char* weapon_get_tooltip(i32 id);

vec2 player_position(void);
f32 player_health(void);
f32 player_mana(void);
f32 player_souls(void);
f32 player_max_health(void);
f32 player_max_mana(void);
f32 player_max_souls(void);

void game_key_callback(i32 key, i32 scancode, i32 action, i32 mods);
void game_mouse_button_callback(i32 button, i32 action, i32 mods);
void game_control_callback(ControlEnum ctrl, i32 action);
void game_framebuffer_size_callback(void);

void camera_set_defaults(Camera* camera);
vec3 camera_get_position(void);
vec2 camera_get_target_position(void);
vec3 camera_get_facing(void);
f32  camera_get_pitch(void);
f32  camera_get_yaw(void);
f32  camera_get_pitch(void);
f32  camera_get_zoom(void);
bool camera_toggle_lock(void);
void camera_set_target(vec2 target);

#endif
