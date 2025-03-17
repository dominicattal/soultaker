/* 
 * On opengl context thread
*/

#include "internal.h"
#include "../window.h"

extern GameContext game_context;

void game_process_input(f32 dt)
{
    vec2 move_mag = vec2_create(0, 0);
    if (window_get_key(GLFW_KEY_W) == GLFW_PRESS)
        move_mag.x += 1;
    if (window_get_key(GLFW_KEY_S) == GLFW_PRESS)
        move_mag.x -= 1;
    if (window_get_key(GLFW_KEY_A) == GLFW_PRESS)
        move_mag.y -= 1;
    if (window_get_key(GLFW_KEY_D) == GLFW_PRESS)
        move_mag.y += 1;
    if (window_get_key(GLFW_KEY_Q) == GLFW_PRESS)
        camera_rotate(1, dt);
    if (window_get_key(GLFW_KEY_E) == GLFW_PRESS)
        camera_rotate(-1, dt);
    if (window_get_key(GLFW_KEY_T) == GLFW_PRESS)
        camera_tilt(1, dt);
    if (window_get_key(GLFW_KEY_Y) == GLFW_PRESS)
        camera_tilt(-1, dt);
    camera_move(move_mag);
}

