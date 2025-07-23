/* 
 * On opengl context thread
*/

#include "internal.h"
#include "../window.h"
#include "../event.h"

extern GameContext game_context;

void game_process_input(f32 dt)
{
    if (game_context.halt_input)
        return;
    if (game_context.halt_game_loop)
        return;

    vec2 move_mag = vec2_create(0, 0);
    f32 rotate_mag = 0;
    f32 tilt_mag = 0; 
    game_context.player.shooting = false;
    if (window_get_key(GLFW_KEY_W) == GLFW_PRESS)
        move_mag.x += 1;
    if (window_get_key(GLFW_KEY_S) == GLFW_PRESS)
        move_mag.x -= 1;
    if (window_get_key(GLFW_KEY_A) == GLFW_PRESS)
        move_mag.y -= 1;
    if (window_get_key(GLFW_KEY_D) == GLFW_PRESS)
        move_mag.y += 1;
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
    event_create_game_camera_move(vec2_scale(vec2_normalize(move_mag), dt));
    camera_rotate(rotate_mag * dt);
    camera_tilt(tilt_mag * dt);
}

