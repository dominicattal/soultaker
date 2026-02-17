#include "../game.h"
#include "../window.h"
#include "../event.h"
#include <math.h>

extern GameContext game_context;

vec2 game_get_player_position(void)
{
    log_assert(game_context.player.entity != NULL, "");
    return game_context.player.entity->position;
}

vec2 game_get_player_direction(void)
{
    log_assert(game_context.player.entity != NULL, "");
    return game_context.player.entity->direction;
}

vec2 game_get_player_facing(void)
{
    log_assert(game_context.player.entity != NULL, "");
    return game_context.player.entity->facing;
}

void game_set_player_position(vec2 position)
{
    log_assert(game_context.player.entity != NULL, "");
    game_context.player.entity->position = position;
}

void inventory_destroy(Inventory* inventory)
{
    for (i32 i = 0; i < inventory->num_items; i++) {
        item_destroy(inventory->items[i]);
        inventory->items[i] = NULL;
    }
}

void player_cleanup(Player* player)
{
    inventory_destroy(&player->inventory);
}

void player_reset(Entity* entity)
{
    Player* player = &game_context.player;
    inventory_destroy(&player->inventory);
    if (player->entity != NULL) {
        log_write(WARNING, "Did not destroy player entity before resetting");
        entity_destroy(player->entity);
    }
    Inventory* inventory = &player->inventory;
    inventory->num_items = 25;
    entity->id = entity_get_id("knight");
    player->entity = entity;
    entity->direction = vec2_create(0, 0);
    entity->size = 1.0;
    entity->speed = 20;
    entity->frame_speed = 2;
    entity->health = entity->max_health = 100000;
    entity_set_flag(entity, ENTITY_FLAG_FRIENDLY, true);
    entity_set_flag(entity, ENTITY_FLAG_PLAYER, true);
    inventory->items[0] = item_create(ITEM_WEAPON, item_get_id("pointer"));
    inventory->items[1] = item_create(ITEM_WEAPON, item_get_id("null_pointer"));
    inventory->items[2] = item_create(ITEM_WEAPON, item_get_id("null_pointer"));
    inventory->item_weapon = &inventory->items[0];
    inventory->item_weapon_swap = &inventory->items[1];
    //player->inventory.weapon.weapon.id = weapon_get_id("pointer");
    //player->inventory.weapon_swap.weapon.id = weapon_get_id("null_pointer");
    gui_update_weapon_info((*inventory->item_weapon)->id);
    player->state_idle = entity_get_state_id(entity, "idle");
    player->state_walking = entity_get_state_id(entity, "walking");
    player->state_shooting = entity_get_state_id(entity, "shooting");
}

vec2 game_get_nearest_player_position(void)
{
    if (game_context.player.entity == NULL)
        return vec2_create(0, 0);
    return game_context.player.entity->position;
}

static void update_player_state(Player* player, f32 dt)
{
    Entity* entity = player->entity;
    if (entity == NULL) return;
    player->shot_timer -= dt;
    player_shoot(player);
    if (player_is_shooting()) {
        if (entity->state == player->state_shooting)
            return;
        entity->state = player->state_shooting;
        entity->frame = 0;
        entity->frame_timer = 0;
    } else {
        if (entity_get_flag(entity, ENTITY_FLAG_MOVING)) {
            if (entity->state == player->state_walking)
                return;
            entity->state = player->state_walking;
            entity->frame = 0;
            entity->frame_timer = 0;
        } else {
            if (entity->state == player->state_idle)
                return;
            entity->state = player->state_idle;
            entity->frame = 0;
            entity->frame_timer = 0;
        }
    }
}

static void update_player_stats(Player* player)
{
    Entity* entity = player->entity;
    if (entity == NULL) {
        player->stats.health = 0;
        player->stats.mana = 0;
        player->stats.souls = 0;
        return;
    }
    player->stats.health = entity->health;
    player->stats.max_health = entity->max_health;
    player->stats.mana = 50;
    player->stats.max_mana = 100;
    player->stats.souls = 50;
    player->stats.max_souls = 100;
}

void player_update(Player* player, f32 dt)
{
    if (player->entity != NULL) {
        player->position = player->entity->position;
        map_fog_explore(game_context.current_map, player->position);
    }
    update_player_state(player, dt);
    update_player_stats(player);
}

void player_swap_weapons(void)
{
    //Inventory* inventory = &game_context.player.inventory;
    //Item* tmp = inventory->items[0];
    //inventory->items[0] = inventory->items[1];
    //inventory->items[1] = tmp;
    //gui_update_weapon_info(inventory->items[0]->id);
}

void player_shoot(Player* player)
{
    if (player->entity == NULL)
        return;
    if (!player->shooting)
        return;
    if (player_is_shooting())
        return;
    player->shot_timer = 0.5;
    vec2 cursor_position = window_cursor_position();
    cursor_position.x /= window_width();
    cursor_position.y /= window_height();
    cursor_position.y = 1 - cursor_position.y;
    f32 ar = window_aspect_ratio();
    f32 zoom = camera_get_zoom();
    f32 tilt = camera_get_pitch();
    f32 rotation = camera_get_yaw();
    f32 a, b, c, r, ratio;
    vec2 direction, target;
    const float character_offset = 1.0 / 4.0 / zoom;
    vec2 pos = vec2_create((cursor_position.x - 0.5) * ar, cursor_position.y - 0.5 + character_offset);
    // https://www.desmos.com/calculator/a7186fd475
    // think of circle as screen space and ellipse inside of it as game space
    // the solution is the intersection point on the ellipse based on angle of screen space
    r = vec2_mag(pos);
    a = atan(pos.y/pos.x);
    b = PI/2 - tilt;
    c = tan(a) * tan(a) / (cos(b) * cos(b)) + 1;
    ratio = sqrt(r*r/(c * cos(a) * cos(a)));
    pos.x =  pos.x > 0 ? 1 / sqrt(c) : -1 / sqrt(c);
    pos.y = -pos.y > 0 ? sqrt(1 - 1 / c) : -sqrt(1 - 1 / c);
    direction.x = pos.x * cos(rotation - HALFPI) - pos.y * sin(rotation - HALFPI);
    direction.y = pos.x * sin(rotation - HALFPI) + pos.y * cos(rotation - HALFPI);
    direction = vec2_normalize(direction);
    target = vec2_sub(player->entity->position, vec2_scale(direction, -2 * zoom * r * r / ratio));
    weapon_shoot(player, direction, target);
    player->entity->facing = direction;
}

bool player_is_shooting(void)
{
    return game_context.player.shot_timer > 0;
}

vec2 player_position(void)
{
    return game_context.player.position;
}

f32 player_health(void)
{
    return game_context.player.stats.health;
}

f32 player_mana(void)
{
    return game_context.player.stats.mana;
}

f32 player_souls(void)
{
    return game_context.player.stats.souls;
}

f32 player_max_health(void)
{
    return game_context.player.stats.max_health;
}

f32 player_max_mana(void)
{
    return game_context.player.stats.max_mana;
}

f32 player_max_souls(void)
{
    return game_context.player.stats.max_souls;
}
