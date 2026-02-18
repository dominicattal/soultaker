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
    player->cast_timer -= dt;
    player_shoot(player);
    player_cast(player);
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

static void player_target(Player* player, f32 height, void (*callback)(Player*, vec2, vec2))
{
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
    f32 vertical_offset = 1.0 / (2.0 * zoom) * height;
    vec2 pos = vec2_create((cursor_position.x - 0.5) * ar, cursor_position.y - 0.5 + vertical_offset);
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
    callback(player, direction, target);
    player->entity->facing = direction;
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
    player_target(player, 0.5, weapon_shoot);
}

static void other_part_update(GameApi* api, Particle* part, f32 dt)
{
    part->size -= 0.4*dt;
    if (part->size < 0) 
        part->size = 0;
}

static void create_aoe(vec2 position)
{
    Particle* part;
    vec2 dir;
    vec3 pos3;
    i32 i;
    f32 rad, hue, lifetime;
    log_write(DEBUG, "Cast");
    AOE* aoe = map_create_aoe(position, 1.0f);
    if (aoe == NULL)
        return;
    i32 n = 100;
    lifetime = 0.4;
    pos3 = vec3_create(position.x, 0.5, position.z);
    for (i = 0; i < n; i++) {
        rad = (2*PI*i)/n;
        dir = vec2_direction(rad);
        part = map_create_particle(pos3);
        if (part == NULL)
            return;
        part->velocity.x = 5*dir.x;
        part->velocity.z = 5*dir.z;
        part->update = other_part_update;
        part->lifetime = lifetime;
        hue = randf();
        part->color.x = hue;
        part->color.y = hue;
        part->color.z = hue;
        part = map_create_particle(pos3);
        if (part == NULL)
            return;
        part->velocity.x = 7*dir.x;
        part->velocity.z = 7*dir.z;
        part->update = other_part_update;
        part->lifetime = lifetime;
        hue = randf();
        part->color.x = hue;
        part->color.y = hue;
        part->color.z = hue;
        part = map_create_particle(pos3);
        if (part == NULL)
            return;
        part->velocity.x = 9*dir.x;
        part->velocity.z = 9*dir.z;
        part->lifetime = lifetime;
        part->update = other_part_update;
        hue = randf();
        part->color.x = hue;
        part->color.y = hue;
        part->color.z = hue;
    }
    //for (i = 0; i < 1000; i++) {
    //    rad = randf() * 2 * PI;
    //    dis = randf_range(2.5, 3.0);
    //    pos = vec2_direction(rad);
    //    pos = vec2_scale(pos, dis);
    //    pos3.x = aoe->position.x;
    //    pos3.y = 0.5;
    //    pos3.z = aoe->position.z;
    //    part->velocity = vec3_create(10*pos.x, 0, 10*pos.z);
    //    part->lifetime = 0.1f;
    //    hue = randf();
    //    part->color.x = hue;
    //    part->color.y = hue;
    //    part->color.z = hue;
    //}
}

static void update_lob(GameApi* api, Particle* part, f32 dt)
{
    Particle* new_part;
    f32* timer = part->data;
    *timer -= dt;
    if (*timer < 0) {
        new_part = map_create_particle(part->position);
        new_part->color.x = new_part->color.y = new_part->color.z = randf();
        new_part->lifetime = 0.1;
        *timer += 0.01;
    }
}

static void destroy_lob(GameApi* api, Particle* part)
{
    create_aoe(vec2_create(part->position.x, part->position.z));
    st_free(part->data);
}

static void cast(Player* player, vec2 direction, vec2 target)
{
    // https://www.desmos.com/calculator/lybiehprmk
    Particle* part;
    vec2 origin = player->entity->position;
    vec2 offset = vec2_sub(target, origin);
    vec2 part_velocity = vec2_normalize(offset);
    f32 distance = vec2_mag(offset);
    vec3 origin3 = vec3_create(origin.x, 0.5, origin.z);
    log_write(DEBUG, "Cast");
    part = map_create_particle(origin3);
    if (part == NULL)
        return;

    f32 g, h, y1, y2, t1, t2, speed;
    g = GRAVITY;
    h = 3.0f;
    y2 = 0.5;
    t1 = sqrt(2*(y2-h)/g);
    y1 = -g*t1;
    t2 = (-y1-sqrt(y1*y1-2*g*y2))/g;
    part->lifetime = t2;
    speed = distance / t2;
    part_velocity = vec2_scale(part_velocity, speed);
    part->velocity.x = part_velocity.x;
    part->velocity.y = y1;
    part->velocity.z = part_velocity.z;
    log_write(DEBUG, "%f %f %f %f %f %f %f", g, h, y2, t1, y1, t2, speed);
    part->acceleration.y = GRAVITY;
    part->data = st_malloc(sizeof(f32));
    part->size = 0.2f;
    *((f32*)part->data) = 0.0f;
    part->update = update_lob;
    part->destroy = destroy_lob;

    //origin3 = vec3_create(target.x, 0, target.z);
    //part = map_create_particle(origin3);

    //Parjicle* parj;
    //parj = map_create_parjicle(origin3);
    //parj->velocity.x = part_velocity.x;
    ////parj->velocity.y = 12.05;
    //parj->velocity.z = part_velocity.z;
    //parj->speed = 3.0;
    ////parj->acceleration.y = GRAVITY;
    //parj->lifetime = mag / parj->speed;
}

void player_cast(Player* player)
{
    if (player->entity == NULL)
        return;
    if (!player->casting)
        return;
    if (player_is_casting())
        return;
    player->cast_timer = 0.5;
    player_target(player, 0.0, cast);
}

bool player_is_shooting(void)
{
    return game_context.player.shot_timer > 0;
}

bool player_is_casting(void)
{
    return game_context.player.cast_timer > 0;
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
