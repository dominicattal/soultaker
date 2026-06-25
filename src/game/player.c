#include "../game.h"
#include "../window.h"
#include "../event.h"
#include <math.h>
#include <string.h>

extern GameContext game_context;

vec2 game_get_player_position(void)
{
    log_assert(game_context.this_client->player.entity != NULL, "");
    return game_context.this_client->player.entity->position;
}

vec2 game_get_player_direction(void)
{
    log_assert(game_context.this_client->player.entity != NULL, "");
    return game_context.this_client->player.entity->direction;
}

vec2 game_get_player_facing(void)
{
    log_assert(game_context.this_client->player.entity != NULL, "");
    return game_context.this_client->player.entity->facing;
}

void game_set_player_position(vec2 position)
{
    log_assert(game_context.this_client->player.entity != NULL, "");
    game_context.this_client->player.entity->position = position;
}

void inventory_init(Client* client)
{
    Inventory* inventory = &client->player.inventory;
    inventory->num_armor_slots = 3;
    inventory->num_weapon_slots = 3;
    inventory->num_ability_slots = 3;
    inventory->num_misc_slots = 25;
    inventory->num_synergies = 0;
    inventory->num_items = inventory->num_armor_slots
                         + inventory->num_weapon_slots
                         + inventory->num_ability_slots
                         + inventory->num_misc_slots;
    inventory->items = st_calloc(inventory->num_items, sizeof(Item*));
    inventory->weapon_slots = st_calloc(inventory->num_weapon_slots, sizeof(Item**));
    inventory->ability_slots = st_calloc(inventory->num_ability_slots, sizeof(Item**));
    inventory->misc_slots = st_calloc(inventory->num_misc_slots, sizeof(Item**));
    inventory->synergies = NULL;

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
}

static void inventory_reset(Client* client)
{
    Inventory* inventory = &client->player.inventory;

    *inventory->misc_slots[0] = item_create(item_get_id("pointer"));
    *inventory->misc_slots[1] = item_create(item_get_id("null_pointer"));
    *inventory->misc_slots[2] = item_create(item_get_id("mothers_pendant"));
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
    *inventory->misc_slots[8] = item_create(item_get_id("robe"));
    *inventory->misc_slots[13] = item_create(item_get_id("wizard_boots"));
    *inventory->misc_slots[4] = item_create(item_get_id("helmet"));
    *inventory->misc_slots[9] = item_create(item_get_id("chestplate"));
    *inventory->misc_slots[14] = item_create(item_get_id("boots"));

    gui_refresh_inventory();
}

void inventory_cleanup(Client* client)
{
    Inventory* inventory = &client->player.inventory;
    //for (i32 i = 0; i < inventory->num_items; i++) {
    //    item_destroy(inventory->items[i]);
    //    inventory->items[i] = NULL;
    //}
    for (i32 i = 0; i < inventory->num_synergies; i++)
        st_free(inventory->synergies[i]);
    st_free(inventory->synergies);
    st_free(inventory->weapon_slots);
    st_free(inventory->ability_slots);
    st_free(inventory->misc_slots);
    st_free(inventory->items);
}

void player_reset(Client* client, Entity* entity)
{
    Player* player = &client->player;
    inventory_reset(client);
    if (player->entity != NULL) {
        log_write(WARNING, "Did not destroy player entity before resetting");
        entity_destroy(player->entity);
    }
    for (i32 i = 0; i < NUM_STATS; i++)
        player->base_stats[i] = 100;
    player->stats[STAT_MP] = 50;
    player->base_stats[STAT_HP_REGEN] = 5;
    player->base_stats[STAT_MP_REGEN] = 5;
    entity->id = entity_get_id("knight");
    player->entity = entity;
    entity->direction = vec2_create(0, 0);
    entity->size = 1.0;
    entity->speed = 20;
    entity->frame_speed = 2;
    entity->health = entity->max_health = player->base_stats[STAT_MAX_HP];
    entity->player = player;
    entity_set_flag(entity, ENTITY_FLAG_FRIENDLY, true);
    entity_set_flag(entity, ENTITY_FLAG_PLAYER, true);
    //entity_set_flag(entity, ENTITY_FLAG_INVULNERABLE, true);
    player->state_idle = entity_get_state_id(entity, "idle");
    player->state_walking = entity_get_state_id(entity, "walking");
    player->state_shooting = entity_get_state_id(entity, "shooting");

    if (client != game_context.this_client) {

        Packet* packet = packet_create(PACKET_SYNC_CLIENT_ENTITY, sizeof(entity->uid), (char*)&entity->uid);
        game_net_send_packet_tcp(client, packet);
        packet_destroy(packet);

        inventory_sync(client);
    }
}

vec2 game_get_nearest_player_position(void)
{
    if (game_context.this_client->player.entity == NULL)
        return vec2_create(0, 0);
    return game_context.this_client->player.entity->position;
}

static void update_player_state(Player* player, f32 dt)
{
    Entity* entity = player->entity;
    if (entity == NULL) return;
    player_shoot_primary(player);
    player_shoot_secondary(player);
    if (player->shooting_primary) {
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

static void update_player_stats(Client* client, f32 dt)
{
    Player* player = &client->player;
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

    if (player != &game_context.this_client->player) {
        size_t size = NUM_STATS * sizeof(f32);
        char* buffer = st_malloc(2 * size);
        memcpy(buffer, player->base_stats, size);
        memcpy(buffer + size, player->stats, size);
        Packet* packet = packet_create(PACKET_CLIENT_STATS, 2 * size, buffer);
        game_net_send_packet_udp(client, packet);
        st_free(buffer);
        packet_destroy(packet);
    }
}

static void update_inventory(Inventory* inventory, f32 dt)
{
    for (i32 i = 0; i < inventory->num_items; i++)
        if (inventory->items[i] != NULL)
            item_update(inventory->items[i], dt);
    for (i32 i = 0; i < inventory->num_synergies; i++)
        if (inventory->synergies[i] != NULL)
            synergy_update(inventory->synergies[i], dt);
}

static void player_update_host(Player* player, f32 dt)
{
    if (player->entity != NULL) {
        player->position = player->entity->position;
        map_fog_explore(game_context.current_map, player->position);
    }
    for (i32 i = 0; i < game_context.clients->length; i++) {
        Client* client = list_get(game_context.clients, i);
        update_player_state(&client->player, dt);
        update_player_stats(client, dt);
    }
    update_inventory(&player->inventory, dt);
}

static void player_update_client(Player* player, f32 dt)
{
    if (!player->synced && game_context.uid_map_type[player->entity_uid] == GAME_OBJ_ENTITY) {
        player->synced = true;
        game_context.this_client->player.entity = game_context.uid_map[player->entity_uid];
        log_write(DEBUG, "synced");
    }
}

void player_update(Player* player, f32 dt)
{
    if (game_context.hosting || game_context.singleplayer)
        player_update_host(player, dt);
    else
        player_update_client(player, dt);
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

void player_shoot_primary(Player* player)
{
    if (player->entity == NULL)
        return;
    if (!player->shooting_primary)
        return;
    player_target(player, 0.5, inventory_shoot_weapons_primary);
}

void player_shoot_secondary(Player* player)
{
    if (player->entity == NULL)
        return;
    if (!player->shooting_secondary)
        return;
    player_target(player, 0.5, inventory_shoot_weapons_secondary);
}

void player_cast(Player* player)
{
    if (player->entity == NULL)
        return;
    if (player->stats[STAT_MP] < 5) {
        log_write(DEBUG, "Not enough mana");
        return;
    }
    player_target(player, 0.0, inventory_cast_abilities);
}

vec2 player_position(void)
{
    return game_context.this_client->player.position;
}

f32 player_health(void)
{
    return game_context.this_client->player.stats[STAT_HP];
}

f32 player_mana(void)
{
    return game_context.this_client->player.stats[STAT_MP];
}

f32 player_souls(void)
{
    return game_context.this_client->player.stats[STAT_SP];
}

f32 player_max_health(void)
{
    return game_context.this_client->player.stats[STAT_MAX_HP];
}

f32 player_max_mana(void)
{
    return game_context.this_client->player.stats[STAT_MAX_MP];
}

f32 player_max_souls(void)
{
    return game_context.this_client->player.stats[STAT_MAX_SP];
}
