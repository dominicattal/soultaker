#include "../game.h"
#include "../window.h"
#include "../event.h"

extern GameContext game_context;

void game_update_keys(void)
{
    static i32 keys[] = {
        GLFW_KEY_W,
        GLFW_KEY_S,
        GLFW_KEY_A,
        GLFW_KEY_D,
        GLFW_KEY_Q,
        GLFW_KEY_E,
        GLFW_KEY_T,
        GLFW_KEY_Y,
    };
    static i32 mbs[] = {
        GLFW_MOUSE_BUTTON_LEFT,
        GLFW_MOUSE_BUTTON_RIGHT,
    };
    for (i32 i = 0; i < (i32)(sizeof(keys) / sizeof(i32)); i++)
        game_context.glfw_key_down[keys[i]] = window_get_key(keys[i]);
    for (i32 i = 0; i < (i32)(sizeof(mbs) / sizeof(i32)); i++)
        game_context.glfw_key_down[mbs[i]] = window_get_mouse_button(mbs[i]);
}

void game_process_input(f32 dt)
{
    vec2 move_mag = vec2_create(0, 0);
    f32 rotate_mag = 0;
    f32 tilt_mag = 0; 
    game_context.this_client->player.shooting_primary = false;
    game_context.this_client->player.shooting_secondary = false;

    if (game_context.halt_input)
        return;

    if (game_context.glfw_key_down[GLFW_KEY_W])
        move_mag.x += 1;
    if (game_context.glfw_key_down[GLFW_KEY_S])
        move_mag.x -= 1;
    if (game_context.glfw_key_down[GLFW_KEY_A])
        move_mag.z -= 1;
    if (game_context.glfw_key_down[GLFW_KEY_D])
        move_mag.z += 1;
    if (game_context.glfw_key_down[GLFW_KEY_Q])
        rotate_mag += 1;
    if (game_context.glfw_key_down[GLFW_KEY_E])
        rotate_mag -= 1;
    if (game_context.glfw_key_down[GLFW_KEY_T])
        tilt_mag += 1;
    if (game_context.glfw_key_down[GLFW_KEY_Y])
        tilt_mag -= 1;
    if (game_context.glfw_key_down[GLFW_MOUSE_BUTTON_LEFT])
        game_context.this_client->player.shooting_primary = true;
    if (game_context.glfw_key_down[GLFW_MOUSE_BUTTON_RIGHT])
        game_context.this_client->player.shooting_secondary = true;

    camera_update_direction(game_context.this_client->uid, move_mag, dt);
    camera_update_rotation(game_context.this_client->uid, rotate_mag);
    camera_update_tilt(game_context.this_client->uid, tilt_mag);
}

