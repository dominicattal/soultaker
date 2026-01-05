#ifndef GAME_H
#define GAME_H

#include "util.h"

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
typedef struct Trigger Trigger;
typedef struct Map Map;
typedef struct MapNode MapNode;
typedef struct MapInfo MapInfo;
typedef struct LocalMapGenerationSettings LocalMapGenerationSettings;

typedef void (*TriggerEnterFunc)(GameApi*, Trigger*, Entity*);
typedef void (*TriggerStayFunc)(GameApi*, Trigger*, Entity*);
typedef void (*TriggerLeaveFunc)(GameApi*, Trigger*, Entity*);
typedef void (*TriggerDestroyFunc)(GameApi*, Trigger*);

typedef void (*InteractableFuncPtr)(GameApi*);

//**************************************************************************
// Core / Thread safe functions
//**************************************************************************

i32 entity_get_id(const char* name);
i32 map_get_id(const char* name);

f32 game_get_dt(void);
f32 game_get_boss_health(void);
f32 game_get_boss_max_health(void);

void game_halt_loop(void);
void game_resume_loop(void);
void game_halt_input(void);
void game_resume_input(void);
void game_halt_render(void);
void game_resume_render(void);

void game_init(void);
void game_cleanup(void);
void game_process_input();
void game_render(void);
void game_load_starting_area(void);

char* weapon_get_name(i32 id);
char* weapon_get_tooltip(i32 id);
i32 weapon_get_tex_id(i32 id);

vec2 player_position(void);
f32 player_health(void);
f32 player_mana(void);
f32 player_souls(void);
f32 player_max_health(void);
f32 player_max_mana(void);
f32 player_max_souls(void);

void game_key_callback(i32 key, i32 scancode, i32 action, i32 mods);
void game_mouse_button_callback(i32 button, i32 action, i32 mods);
void game_framebuffer_size_callback(void);

vec3 camera_get_position(void);
vec3 camera_get_facing(void);
f32  camera_get_pitch(void);
f32  camera_get_yaw(void);
f32  camera_get_pitch(void);
f32  camera_get_zoom(void);

#endif
