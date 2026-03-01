#include "../game.h"
#include "../window.h"
#include "../event.h"

extern GameContext game_context;

void game_process_input()
{
    if (game_context.halt_game_loop)
        return;

    vec2 move_mag = vec2_create(0, 0);
    f32 rotate_mag = 0;
    f32 tilt_mag = 0; 
    game_context.player.shooting = false;

    if (game_context.halt_input)
        goto update;

    if (window_get_key(GLFW_KEY_W) == GLFW_PRESS)
        move_mag.x += 1;
    if (window_get_key(GLFW_KEY_S) == GLFW_PRESS)
        move_mag.x -= 1;
    if (window_get_key(GLFW_KEY_A) == GLFW_PRESS)
        move_mag.z -= 1;
    if (window_get_key(GLFW_KEY_D) == GLFW_PRESS)
        move_mag.z += 1;
    if (window_get_key(GLFW_KEY_Q) == GLFW_PRESS)
        rotate_mag += 1;
    if (window_get_key(GLFW_KEY_E) == GLFW_PRESS)
        rotate_mag -= 1;
    if (window_get_key(GLFW_KEY_T) == GLFW_PRESS)
        tilt_mag += 1;
    if (window_get_key(GLFW_KEY_Y) == GLFW_PRESS)
        tilt_mag -= 1;
    if (window_get_mouse_button(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        game_context.player.shooting = true;
    //if (window_get_mouse_button(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    //    game_context.player.casting = true;

update:
    event_create_game_camera_update_direction(move_mag);
    event_create_game_camera_update_rotation(rotate_mag);
    event_create_game_camera_update_tilt(tilt_mag);
}

