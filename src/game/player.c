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

void inventory_init(Inventory* inventory)
{
    inventory->num_armor_slots = 3;
    inventory->num_weapon_slots = 3;
    inventory->num_ability_slots = 3;
    inventory->num_misc_slots = 25;
    inventory->num_items = inventory->num_armor_slots
                         + inventory->num_weapon_slots
                         + inventory->num_ability_slots
                         + inventory->num_misc_slots;
    inventory->items = st_calloc(inventory->num_items, sizeof(Item*));
    inventory->weapon_slots = st_calloc(inventory->num_weapon_slots, sizeof(Item**));
    inventory->ability_slots = st_calloc(inventory->num_ability_slots, sizeof(Item**));
    inventory->misc_slots = st_calloc(inventory->num_misc_slots, sizeof(Item**));

    inventory->armor_slots[0]   = &inventory->items[0];
    inventory->armor_slots[1]   = &inventory->items[1];
    inventory->armor_slots[2]   = &inventory->items[2];
    inventory->weapon_slots[0]  = &inventory->items[3];
    inventory->weapon_slots[1]  = &inventory->items[4];
    inventory->weapon_slots[2]  = &inventory->items[5];
    inventory->ability_slots[0] = &inventory->items[6];
    inventory->ability_slots[1] = &inventory->items[7];
    inventory->ability_slots[2] = &inventory->items[8];

    for (i32 i = 0; i < inventory->num_misc_slots; i++)
        inventory->misc_slots[i] = &inventory->items[i+9];

    *inventory->misc_slots[0] = item_create(item_get_id("pointer"));
    *inventory->misc_slots[1] = item_create(item_get_id("null_pointer"));
    *inventory->misc_slots[2] = item_create(item_get_id("mothers_pendant"));
    (*inventory->misc_slots[2])->additive_stats[STAT_MAX_HP] = 10;
    *inventory->misc_slots[5] = item_create(item_get_id("shiv"));
    *inventory->misc_slots[6] = item_create(item_get_id("staff"));
    *inventory->misc_slots[7] = item_create(item_get_id("wand"));
    *inventory->misc_slots[10] = item_create(item_get_id("spelltome"));
    *inventory->misc_slots[11] = item_create(item_get_id("healing_tome"));
    *inventory->misc_slots[12] = item_create(item_get_id("hermes_boots"));

    *inventory->misc_slots[15] = item_create(item_get_id("feral_claws"));
    *inventory->misc_slots[16] = item_create(item_get_id("bear_hide"));
    *inventory->misc_slots[17] = item_create(item_get_id("dragon_scale"));

    *inventory->misc_slots[3] = item_create(item_get_id("wizard_hat"));
    (*inventory->misc_slots[3])->additive_stats[STAT_MAX_MP] = 50;
    *inventory->misc_slots[8] = item_create(item_get_id("robe"));
    (*inventory->misc_slots[8])->additive_stats[STAT_MAX_MP] = 50;
    *inventory->misc_slots[13] = item_create(item_get_id("wizard_boots"));
    (*inventory->misc_slots[13])->additive_stats[STAT_MAX_MP] = 50;

    *inventory->misc_slots[4] = item_create(item_get_id("helmet"));
    (*inventory->misc_slots[4])->additive_stats[STAT_MAX_HP] = 50;
    *inventory->misc_slots[9] = item_create(item_get_id("chestplate"));
    (*inventory->misc_slots[9])->additive_stats[STAT_MAX_HP] = 50;
    *inventory->misc_slots[14] = item_create(item_get_id("boots"));
    (*inventory->misc_slots[14])->additive_stats[STAT_MAX_HP] = 50;

    gui_refresh_inventory();
}

void inventory_destroy(Inventory* inventory)
{
    for (i32 i = 0; i < inventory->num_items; i++) {
        item_destroy(inventory->items[i]);
        inventory->items[i] = NULL;
    }
    st_free(inventory->weapon_slots);
    st_free(inventory->ability_slots);
    st_free(inventory->misc_slots);
    st_free(inventory->items);
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
    for (i32 i = 0; i < NUM_STATS; i++)
        player->base_stats[i] = 100;
    player->stats[STAT_MP] = 50;
    player->base_stats[STAT_HP_REGEN] = 5;
    player->base_stats[STAT_MP_REGEN] = 5;
    inventory_init(&player->inventory);
    entity->id = entity_get_id("knight");
    player->entity = entity;
    entity->direction = vec2_create(0, 0);
    entity->size = 1.0;
    entity->speed = 20;
    entity->frame_speed = 2;
    entity->health = entity->max_health = player->base_stats[STAT_MAX_HP];
    entity_set_flag(entity, ENTITY_FLAG_FRIENDLY, true);
    entity_set_flag(entity, ENTITY_FLAG_PLAYER, true);
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

static void update_player_stats(Player* player, f32 dt)
{
    Entity* entity = player->entity;
    if (entity == NULL) {
        player->stats[STAT_HP] = 0;
        player->stats[STAT_MP] = 0;
        player->stats[STAT_SP] = 0;
        return;
    }
    for (i32 j = 0; j < NUM_STATS; j++) {
        if (j == STAT_MP) continue;
        if (j == STAT_HP) continue;
        if (j == STAT_SP) continue;
        player->stats[j] = player->base_stats[j];
    }
    for (i32 i = 0; i < 25; i++) {
        Item* item = player->inventory.items[i];
        if (item == NULL) 
            continue;
        if (!item->equipped)
            continue;
        for (i32 j = 0; j < NUM_STATS; j++)
            player->stats[j] += item->additive_stats[j];
    }

    entity->max_health = player->stats[STAT_MAX_HP];
    player->entity->health += player->stats[STAT_HP_REGEN] * dt;
    if (player->entity->health > player->entity->max_health)
        player->entity->health = player->entity->max_health;
    player->stats[STAT_HP] = entity->health;
    player->stats[STAT_MAX_HP] = entity->max_health;

    player->stats[STAT_MP] += player->stats[STAT_MP_REGEN] * dt;
    if (player->stats[STAT_MP] > player->stats[STAT_MAX_MP])
        player->stats[STAT_MP] = player->stats[STAT_MAX_MP];
    player->stats[STAT_SP] = 50;
    player->stats[STAT_MAX_SP] = 100;
}

void player_update(Player* player, f32 dt)
{
    if (player->entity != NULL) {
        player->position = player->entity->position;
        map_fog_explore(game_context.current_map, player->position);
    }
    update_player_state(player, dt);
    update_player_stats(player, dt);
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
    if (player->stats[STAT_MP] < 5) {
        log_write(DEBUG, "Not enough mana");
        return;
    }
    player->cast_timer = 0.5;
    player_target(player, 0.0, cast);
    player->stats[STAT_MP] -= 5;
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
    return game_context.player.stats[STAT_HP];
}

f32 player_mana(void)
{
    return game_context.player.stats[STAT_MP];
}

f32 player_souls(void)
{
    return game_context.player.stats[STAT_SP];
}

f32 player_max_health(void)
{
    return game_context.player.stats[STAT_MAX_HP];
}

f32 player_max_mana(void)
{
    return game_context.player.stats[STAT_MAX_MP];
}

f32 player_max_souls(void)
{
    return game_context.player.stats[STAT_MAX_SP];
}
