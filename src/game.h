#ifndef GAME_H
#define GAME_H

#include "util.h"

void game_init(void);
void game_cleanup(void);
void game_process_input(f32 dt);
void game_render_init(void);
void game_render(void);
void game_render_cleanup(void);
void game_key_callback(i32 key, i32 scancode, i32 action, i32 mods);
void game_mouse_button_callback(i32 button, i32 action, i32 mods);
void game_framebuffer_size_callback(void);
void game_halt_input(void);
void game_resume_input(void);

f32 game_dt(void);

vec3 camera_get_position(void);
vec3 camera_get_facing(void);
f32  camera_get_pitch(void);
f32  camera_get_yaw(void);
f32  camera_get_pitch(void);
f32  camera_get_zoom(void);

#endif
