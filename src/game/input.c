#include "../game.h"
#include "../window.h"
#include "../event.h"
#include <string.h>

extern GameContext game_context;

void game_update_keys(void)
{
    Client* client = game_context.this_client;
    client->control_flags = 0;
    if (window_get_key(GLFW_KEY_W))
        client->control_flags |= INPUT_W;
    if (window_get_key(GLFW_KEY_S))
        client->control_flags |= INPUT_S;
    if (window_get_key(GLFW_KEY_A))
        client->control_flags |= INPUT_A;
    if (window_get_key(GLFW_KEY_D))
        client->control_flags |= INPUT_D;
    if (window_get_key(GLFW_KEY_Q))
        client->control_flags |= INPUT_Q;
    if (window_get_key(GLFW_KEY_E))
        client->control_flags |= INPUT_E;
    if (window_get_key(GLFW_KEY_T))
        client->control_flags |= INPUT_T;
    if (window_get_key(GLFW_KEY_Y))
        client->control_flags |= INPUT_Y;
    if (window_get_mouse_button(GLFW_MOUSE_BUTTON_LEFT))
        client->control_flags |= INPUT_MB_LEFT;
    if (window_get_mouse_button(GLFW_MOUSE_BUTTON_RIGHT))
        client->control_flags |= INPUT_MB_RIGHT;
}

static void client_process_input(Client* client, f32 dt)
{
    vec2 move_mag = vec2_create(0, 0);
    client->player.shooting_primary = false;
    client->player.shooting_secondary = false;

    if (client->control_flags & INPUT_W)
        move_mag.x += 1;
    if (client->control_flags & INPUT_S)
        move_mag.x -= 1;
    if (client->control_flags & INPUT_A)
        move_mag.z -= 1;
    if (client->control_flags & INPUT_D)
        move_mag.z += 1;
    if (client->control_flags & INPUT_MB_LEFT)
        client->player.shooting_primary = true;
    if (client->control_flags & INPUT_MB_RIGHT)
        client->player.shooting_secondary = true;

    camera_update_direction(client->uid, move_mag, dt);
}

void game_process_input(f32 dt)
{
    Client* client = game_context.this_client;
    f32 rotate_mag = 0;
    f32 tilt_mag = 0; 

    if (game_context.halt_input)
        return;

    if (client->control_flags & INPUT_Q)
        rotate_mag += 1;
    if (client->control_flags & INPUT_E)
        rotate_mag -= 1;
    if (client->control_flags & INPUT_T)
        tilt_mag += 1;
    if (client->control_flags & INPUT_Y)
        tilt_mag -= 1;

    camera_update_rotation(client->uid, rotate_mag);
    camera_update_tilt(client->uid, tilt_mag);

    if (game_context.hosting || game_context.singleplayer) {
        for (i32 i = 0; i < game_context.clients->length; i++) {
            client = list_get(game_context.clients, i);
            client_process_input(client, dt);
        }
    }
}

