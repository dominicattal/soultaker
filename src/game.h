#ifndef GAME_H
#define GAME_H

#include "util.h"

i32 game_preset_map_id(const char* name);

f32 game_get_dt(void);
f32 game_get_boss_health(void);
f32 game_get_boss_max_health(void);

void game_init(void);
void game_halt_loop(void);
void game_resume_loop(void);
void game_cleanup(void);
void game_process_input(f32 dt);
void game_render(void);
void game_halt_input(void);
void game_resume_input(void);
void game_load_starting_area(void);

char* weapon_get_name(i32 id);
char* weapon_get_tooltip(i32 id);
i32 weapon_get_tex_id(i32 id);

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
