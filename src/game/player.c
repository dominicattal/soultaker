#include "internal.h"
#include "../window.h"
#include <math.h>

extern GameContext game_context;

void player_update(Player* player, f32 dt)
{
    player->shot_timer -= dt;
    player_shoot(player);
}

void weapon_shoot(Player* player, vec3 direction, vec3 target)
{
    vec3 pos = player->entity->position;
    pos.y = 0.5;
    projectile_create(pos);
}

void player_shoot(Player* player)
{
    if (player->entity == NULL)
        return;
    if (!player->shooting)
        return;
    if (player->shot_timer > 0)
        return;
    player->shot_timer = 0.5;
    vec2 cursor_position = window_cursor_position();
    f32 ar = window_aspect_ratio();
    f32 zoom = camera_get_zoom();
    f32 tilt = camera_get_pitch();
    f32 rotation = camera_get_yaw();
    f32 dirx, dirz, a, b, c, r, ratio;
    vec3 direction, target;
    vec2 pos = vec2_create((cursor_position.x - 0.5) * ar, cursor_position.y - 0.5 + 1.0 / 4 / zoom);
    r = vec2_mag(pos);
    a = atan(pos.y/pos.x);
    b = PI/2 - tilt;
    c = tan(a) * tan(a) / (cos(b) * cos(b)) + 1;
    ratio = sqrt(r*r/(c * cos(a) * cos(a)));
    pos.x =  pos.x > 0 ? 1 / sqrt(c) : -1 / sqrt(c);
    pos.y = -pos.y > 0 ? sqrt(1 - 1 / c) : -sqrt(1 - 1 / c);
    dirx = pos.x * cos(rotation - HALFPI) - pos.y * sin(rotation - HALFPI);
    dirz = pos.x * sin(rotation - HALFPI) + pos.y * cos(rotation - HALFPI);
    direction = vec3_normalize(vec3_create(dirx, 0.0, dirz));
    target = vec3_sub(player->entity->position, vec3_scale(direction, -2 * zoom * r * r / ratio));
    weapon_shoot(player, direction, target);
    //player.entity->facing = vec2_normalize(vec2f_create(dirx, dirz));
    //player.entity->flag = TRUE;
}
