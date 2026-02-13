#include "../../src/api.h"
#include <string.h>

typedef struct {
    Entity* boss;
} MapData;

st_export void* outpost1_init(GameApi* api)
{
    MapData* map_data = api->st_malloc(sizeof(MapData));
    map_data->boss = NULL;
    return map_data;
}

st_export void outpost1_cleanup(GameApi* api, void* data)
{
    api->st_free(data);
}

st_export bool outpost1_generate(GameApi* api, LocalMapGenerationSettings* settings)
{
    if (settings->num_rooms_left != 0)
        return false;

    if (strcmp(settings->current_branch, "main") == 0) {
        if (strcmp(settings->current_room_type, "spawn") == 0) {
            settings->current_room_type = "boss";
            //settings->create_no_path = true;
            settings->num_rooms_left = 1;
            return false;
        } else if (strcmp(settings->current_room_type, "enemy") == 0) {
            settings->current_room_type = "boss";
            settings->num_rooms_left = 1;
            return false;
        }
    }

    return true;
}

st_export bool outpost1_branch(GameApi* api, LocalMapGenerationSettings* settings)
{
    return false;
    if (strcmp(settings->current_branch, "main") != 0)
        return false;
    if (rand() % 10 == 0) {
        settings->current_branch = "dead_end";
        settings->current_room_type = "enemy";
        settings->num_rooms_left = 5 + rand() % 3;
        return true;
    }
    return false;
}

st_export void outpost1_big_room_create(GameApi* api)
{
    vec2 position = api->vec2_create(14, 14);
    i32 id;
    id = api->entity_get_id("outpost1_knight");
    api->room_create_entity(position, id);
    //api->room_create_entity(position, id);
    //api->room_create_entity(position, id);
    //id = api->entity_get_id("outpost1_archer");
    //api->room_create_entity(position, id);
    //api->room_create_entity(position, id);
    //api->room_create_entity(position, id);
    //id = api->entity_get_id("outpost1_mage");
    //api->room_create_entity(position, id);
    //api->room_create_entity(position, id);
    //api->room_create_entity(position, id);
}

st_export void outpost1_boss_room_create(GameApi* api)
{
    vec2 position = api->vec2_create(28, 10);
    i32 id;
    id = api->entity_get_id("outpost1_boss");
    api->room_create_entity(position, id);
}

// *************************************************************
// Knight
// *************************************************************

#define KNIGHT_IN_RANGE_THRESHOLD      10
#define KNIGHT_OUT_OF_RANGE_THRESHOLD  20

typedef struct {
    vec2 spawn_point;
    f32 wander_cooldown;
    f32 wander_timer;
    f32 shot_cooldown;
    i32 rotate_direction;
} KnightData;

st_export void outpost1_knight_create(GameApi* api, Entity* entity)
{
    KnightData* data = api->st_malloc(sizeof(KnightData));
    data->spawn_point = entity->position;
    data->wander_cooldown = 0;
    data->shot_cooldown = 0;
    data->rotate_direction = (2*(rand()%2))-1;
    entity->health = entity->max_health = 10;
    entity->size = 1.5f;
    entity->hitbox_radius = 0.7;
    entity->data = data;
    entity->state = api->entity_get_state_id(entity, "idle");
}

st_export void outpost1_knight_destroy(GameApi* api, Entity* entity)
{
    api->st_free(entity->data);
}

static bool outpost1_player_in_range(GameApi* api, Entity* entity, f32 range)
{
    return api->vec2_mag(api->vec2_sub(api->game_get_nearest_player_position(), entity->position)) < range;
}

static bool outpost1_player_out_of_range(GameApi* api, Entity* entity, f32 range)
{
    return api->vec2_mag(api->vec2_sub(api->game_get_nearest_player_position(), entity->position)) > range;
}

st_export void outpost1_knight_idle_update(GameApi* api, Entity* entity, f32 dt)
{
    vec2 offset, target, distance;
    KnightData* data = entity->data;
    if (outpost1_player_in_range(api, entity, KNIGHT_IN_RANGE_THRESHOLD)) {
        entity->state = api->entity_get_state_id(entity, "attack");
        return;
    }
    data->wander_cooldown -= dt;
    if (data->wander_cooldown < 0) {
        offset = api->vec2_create(api->randf() * 3, 0.0f);
        offset = api->vec2_rotate(offset, api->randf() * 2 * PI);
        target = api->vec2_add(data->spawn_point, offset);
        distance = api->vec2_sub(target, entity->position);
        data->wander_timer = api->vec2_mag(distance) / entity->speed;
        entity->direction = api->vec2_normalize(distance);
        entity->state = api->entity_get_state_id(entity, "wander");
    }
}

st_export void outpost1_knight_wander_update(GameApi* api, Entity* entity, f32 dt)
{
    KnightData* data = entity->data;
    if (outpost1_player_in_range(api, entity, KNIGHT_IN_RANGE_THRESHOLD)) {
        entity->state = api->entity_get_state_id(entity, "attack");
        return;
    }
    data->wander_timer -= dt;
    if (data->wander_timer > 0)
        return;
    entity->direction = api->vec2_create(0, 0);
    data->wander_cooldown = api->randf_range(3.0f, 7.0f);
    entity->state = api->entity_get_state_id(entity, "idle");
}

st_export void outpost1_knight_attack_update(GameApi* api, Entity* entity, f32 dt)
{
    KnightData* data = entity->data;
    Projectile* proj;
    vec2 direction, offset, target;
    vec2 player_position = api->game_get_nearest_player_position();
    f64 distance = api->vec2_mag(api->vec2_sub(player_position, entity->position));
    if (outpost1_player_out_of_range(api, entity, KNIGHT_OUT_OF_RANGE_THRESHOLD)) {
        entity->state = api->entity_get_state_id(entity, "idle");
        entity->direction = api->vec2_create(0, 0);
        return;
    }
    direction = api->vec2_normalize(api->vec2_sub(player_position, entity->position));
    if (distance > 3.1) {
        entity->direction = direction;
        return;
    }
    if (api->entity_get_flag(entity, ENTITY_FLAG_HIT_WALL))
        data->rotate_direction = -data->rotate_direction;

    vec2 player_offset = api->vec2_sub(entity->position, player_position);
    f64 player_offset_rad = api->vec2_radians(player_offset);
    f64 player_arc_rad = api->gmodf(player_offset_rad + data->rotate_direction * dt, 2 * PI);
    offset = api->vec2_direction(player_arc_rad);
    offset = api->vec2_scale(offset, 3.0f);
    target = api->vec2_add(player_position, offset);
    entity->direction = api->vec2_normalize(api->vec2_sub(target, entity->position));
    data->shot_cooldown -= dt;
    if (api->entity_get_flag(entity, ENTITY_FLAG_HIT_WALL)) {
        //api->log_write(DEBUG, "%f %f", player_position.x, player_position.z);
        //api->log_write(DEBUG, "%f %f %f %f %f %f", before, data->player_arc_rad, offset.x, offset.z, target.x, target.z);
        //api->log_write(DEBUG, "%f %f %f %f %i", entity->direction.x, entity->direction.z, entity->position.x, entity->position.z, data->rotate_direction);
    }
    if (data->shot_cooldown < 0) {
        proj = api->map_create_projectile(entity->position);
        proj->direction = api->vec2_rotate(direction, api->randf_range(-0.3, 0.3));
        proj->speed = 6.5f;
        proj->size = 0.5f;
        proj->tex = api->texture_get_id("bullet");
        proj->facing = api->vec2_radians(proj->direction);
        data->shot_cooldown += 1.0f;
    }
}

// *************************************************************
// Archer
// *************************************************************

#define ARCHER_IN_RANGE_THRESHOLD      10
#define ARCHER_OUT_OF_RANGE_THRESHOLD  20

typedef struct {
    vec2 spawn_point;
    f32 wander_cooldown;
    f32 wander_timer;
    f32 reposition_timer;
    f32 attack_timer;
    f32 player_arc_rad;
    f32 shot_cooldown;
} ArcherData;

st_export void outpost1_archer_create(GameApi* api, Entity* entity)
{
    ArcherData* data = api->st_malloc(sizeof(ArcherData));
    data->spawn_point = entity->position;
    data->wander_cooldown = 0;
    data->shot_cooldown = 0;
    data->player_arc_rad = 0;
    data->attack_timer = 0;
    entity->health = entity->max_health = 10;
    entity->speed = 5.0f;
    entity->size = 1.5f;
    entity->hitbox_radius = 0.7;
    entity->data = data;
    entity->state = api->entity_get_state_id(entity, "idle");
}

st_export void outpost1_archer_destroy(GameApi* api, Entity* entity)
{
    api->st_free(entity->data);
}

static void reposition_archer(GameApi* api, Entity* entity)
{
    ArcherData* data = entity->data;
    vec2 player_position = api->game_get_nearest_player_position();
    vec2 direction = api->vec2_sub(entity->position, player_position);
    f32 cur_rad = api->vec2_radians(direction);
    cur_rad += (2*(rand()%2)-1) * api->randf_range(0.1, 0.2);
    vec2 offset = api->vec2_scale(api->vec2_direction(cur_rad), 6.5f);
    vec2 target = api->vec2_add(player_position, offset);
    direction = api->vec2_sub(target, entity->position);
    entity->direction = api->vec2_normalize(direction);
    data->reposition_timer = api->vec2_mag(direction) / entity->speed;
}

st_export void outpost1_archer_idle_update(GameApi* api, Entity* entity, f32 dt)
{
    vec2 offset, target, distance;
    ArcherData* data = entity->data;
    if (outpost1_player_in_range(api, entity, ARCHER_IN_RANGE_THRESHOLD)) {
        entity->state = api->entity_get_state_id(entity, "reposition");
        reposition_archer(api, entity);
        return;
    }
    data->wander_cooldown -= dt;
    if (data->wander_cooldown < 0) {
        offset = api->vec2_create(api->randf() * 3, 0.0f);
        offset = api->vec2_rotate(offset, api->randf() * 2 * PI);
        target = api->vec2_add(data->spawn_point, offset);
        distance = api->vec2_sub(target, entity->position);
        data->wander_timer = api->vec2_mag(distance) / entity->speed;
        entity->direction = api->vec2_normalize(distance);
        entity->state = api->entity_get_state_id(entity, "wander");
    }
}

st_export void outpost1_archer_wander_update(GameApi* api, Entity* entity, f32 dt)
{
    ArcherData* data = entity->data;
    if (outpost1_player_in_range(api, entity, ARCHER_IN_RANGE_THRESHOLD)) {
        entity->state = api->entity_get_state_id(entity, "reposition");
        reposition_archer(api, entity);
        return;
    }
    data->wander_timer -= dt;
    if (data->wander_timer > 0)
        return;
    entity->direction = api->vec2_create(0, 0);
    data->wander_cooldown = api->randf_range(3.0f, 7.0f);
    entity->state = api->entity_get_state_id(entity, "idle");
}

st_export void outpost1_archer_attack_update(GameApi* api, Entity* entity, f32 dt)
{
    ArcherData* data = entity->data;
    Projectile* proj;
    vec2 direction, player_position;
    data->attack_timer -= dt;
    if (data->attack_timer < 0) {
        proj = api->map_create_projectile(entity->position);
        player_position = api->game_get_nearest_player_position();
        direction = api->vec2_normalize(api->vec2_sub(player_position, entity->position));
        proj->direction = direction;
        //proj->direction = api->vec2_rotate(direction, api->randf_range(-0.3, 0.3));
        proj->speed = 10.0f;
        proj->size = 0.5f;
        proj->tex = api->texture_get_id("bullet");
        proj->facing = api->vec2_radians(proj->direction);
        data->attack_timer += 0.5f;
    }
    data->reposition_timer -= dt;
    if (data->reposition_timer < 0) {
        entity->state = api->entity_get_state_id(entity, "reposition");
        reposition_archer(api, entity);
    }
}

st_export void outpost1_archer_reposition_update(GameApi* api, Entity* entity, f32 dt)
{
    ArcherData* data = entity->data;
    data->reposition_timer -= dt;
    if (data->reposition_timer < 0) {
        entity->state = api->entity_get_state_id(entity, "attack");
        data->reposition_timer = 3.0f;
    }
}

// *************************************************************
// Mage
// *************************************************************

#define MAGE_IN_RANGE_THRESHOLD      10
#define MAGE_OUT_OF_RANGE_THRESHOLD  20

typedef struct {
    vec2 spawn_point;
    f32 wander_cooldown;
    f32 wander_timer;
    f32 reposition_timer;
    f32 attack_timer;
    f32 player_arc_rad;
    f32 shot_cooldown;
} MageData;

st_export void outpost1_mage_create(GameApi* api, Entity* entity)
{
    ArcherData* data = api->st_malloc(sizeof(ArcherData));
    data->spawn_point = entity->position;
    data->wander_cooldown = 0;
    data->shot_cooldown = 0;
    data->player_arc_rad = 0;
    data->attack_timer = 0;
    entity->health = entity->max_health = 10;
    entity->speed = 5.0f;
    entity->size = 1.5f;
    entity->hitbox_radius = 0.7;
    entity->data = data;
    entity->state = api->entity_get_state_id(entity, "idle");
}

st_export void outpost1_mage_destroy(GameApi* api, Entity* entity)
{
    api->st_free(entity->data);
}

static void reposition_mage(GameApi* api, Entity* entity)
{
    ArcherData* data = entity->data;
    vec2 player_position = api->game_get_nearest_player_position();
    vec2 direction = api->vec2_sub(entity->position, player_position);
    f32 cur_rad = api->vec2_radians(direction);
    cur_rad += (2*(rand()%2)-1) * api->randf_range(0.1, 0.2);
    vec2 offset = api->vec2_scale(api->vec2_direction(cur_rad), 6.5f);
    vec2 target = api->vec2_add(player_position, offset);
    direction = api->vec2_sub(target, entity->position);
    entity->direction = api->vec2_normalize(direction);
    data->reposition_timer = api->vec2_mag(direction) / entity->speed;
}

st_export void outpost1_mage_idle_update(GameApi* api, Entity* entity, f32 dt)
{
    vec2 offset, target, distance;
    ArcherData* data = entity->data;
    if (outpost1_player_in_range(api, entity, ARCHER_IN_RANGE_THRESHOLD)) {
        entity->state = api->entity_get_state_id(entity, "reposition");
        reposition_mage(api, entity);
        return;
    }
    data->wander_cooldown -= dt;
    if (data->wander_cooldown < 0) {
        offset = api->vec2_create(api->randf() * 3, 0.0f);
        offset = api->vec2_rotate(offset, api->randf() * 2 * PI);
        target = api->vec2_add(data->spawn_point, offset);
        distance = api->vec2_sub(target, entity->position);
        data->wander_timer = api->vec2_mag(distance) / entity->speed;
        entity->direction = api->vec2_normalize(distance);
        entity->state = api->entity_get_state_id(entity, "wander");
    }
}

st_export void outpost1_mage_wander_update(GameApi* api, Entity* entity, f32 dt)
{
    ArcherData* data = entity->data;
    if (outpost1_player_in_range(api, entity, ARCHER_IN_RANGE_THRESHOLD)) {
        entity->state = api->entity_get_state_id(entity, "reposition");
        reposition_mage(api, entity);
        return;
    }
    data->wander_timer -= dt;
    if (data->wander_timer > 0)
        return;
    entity->direction = api->vec2_create(0, 0);
    data->wander_cooldown = api->randf_range(3.0f, 7.0f);
    entity->state = api->entity_get_state_id(entity, "idle");
}

static void mage_projectile_update(GameApi* api, Projectile* proj, f32 dt)
{
    vec2 player_position = api->game_get_nearest_player_position();
    vec2 acceleration = api->vec2_normalize(api->vec2_sub(player_position, proj->position));
    vec2 direction = api->vec2_add(proj->direction, api->vec2_scale(acceleration, 3 * dt));
    proj->direction = api->vec2_normalize(direction);
    proj->facing = api->vec2_radians(proj->direction);
}

st_export void outpost1_mage_attack_update(GameApi* api, Entity* entity, f32 dt)
{
    ArcherData* data = entity->data;
    Projectile* proj;
    vec2 direction, player_position;
    data->attack_timer -= dt;
    if (data->attack_timer < 0) {
        proj = api->map_create_projectile(entity->position);
        player_position = api->game_get_nearest_player_position();
        direction = api->vec2_normalize(api->vec2_sub(player_position, entity->position));
        proj->direction = direction;
        //proj->direction = api->vec2_rotate(direction, api->randf_range(-0.3, 0.3));
        proj->speed = 10.0f;
        proj->size = 0.5f;
        proj->tex = api->texture_get_id("bullet");
        proj->facing = api->vec2_radians(proj->direction);
        proj->update = mage_projectile_update;
        data->attack_timer += 0.5f;
    }
    data->reposition_timer -= dt;
    if (data->reposition_timer < 0) {
        entity->state = api->entity_get_state_id(entity, "reposition");
        reposition_mage(api, entity);
    }
}

st_export void outpost1_mage_reposition_update(GameApi* api, Entity* entity, f32 dt)
{
    ArcherData* data = entity->data;
    data->reposition_timer -= dt;
    if (data->reposition_timer < 0) {
        entity->state = api->entity_get_state_id(entity, "attack");
        data->reposition_timer = 3.0f;
    }
}

// *************************************************************
// Boss
// *************************************************************

static void boss_start(GameApi* api, Trigger* trigger, Entity* entity)
{
    MapData* map_data = api->map_get_data();
    api->log_write(DEBUG, "%f %f", entity->position.x, entity->position.z);
    api->log_write(DEBUG, "test");
    if (map_data->boss == NULL)
        api->log_write(FATAL, "Map boss is null");
    api->map_make_boss("Asgore", map_data->boss);
    Wall* wall;
    wall = api->room_set_tilemap_wall(26, 54, 2.0f, 0x683434);
    wall->top_tex = api->texture_get_id("outpost1_wall1_top");
    wall->side_tex = api->texture_get_id("outpost1_wall1_side");
    wall = api->room_set_tilemap_wall(27, 54, 2.0f, 0x683434);
    wall->top_tex = api->texture_get_id("outpost1_wall1_top");
    wall->side_tex = api->texture_get_id("outpost1_wall1_side");
    wall = api->room_set_tilemap_wall(28, 54, 2.0f, 0x683435);
    wall->top_tex = api->texture_get_id("outpost1_wall1_top");
    wall->side_tex = api->texture_get_id("outpost1_wall1_side");
    wall = api->room_set_tilemap_wall(29, 54, 2.0f, 0x683434);
    wall->top_tex = api->texture_get_id("outpost1_wall1_top");
    wall->side_tex = api->texture_get_id("outpost1_wall1_side");
    wall = api->room_set_tilemap_wall(30, 54, 2.0f, 0x683434);
    wall->top_tex = api->texture_get_id("outpost1_wall1_top");
    wall->side_tex = api->texture_get_id("outpost1_wall1_side");
}

st_export void outpost1_boss_create(GameApi* api, Entity* entity)
{
    Trigger* trigger = api->map_create_trigger(entity->position, 10.0f);
    api->trigger_set_flag(trigger, TRIGGER_FLAG_ONCE, true);
    api->trigger_set_flag(trigger, TRIGGER_FLAG_PLAYER, true);
    trigger->enter = boss_start;
    entity->size = 3.0;
    entity->health = entity->max_health = 100;

    MapData* map_data = api->map_get_data();
    map_data->boss = entity;
}

st_export void outpost1_boss_destroy(GameApi* api, Entity* entity)
{
}

st_export void outpost1_boss_idle_update(GameApi* api, Entity* entity, f32 dt)
{
}

st_export void outpost1_boss_wander_update(GameApi* api, Entity* entity, f32 dt)
{
}

st_export void outpost1_boss_attack_update(GameApi* api, Entity* entity, f32 dt)
{
}

st_export void outpost1_boss_reposition_update(GameApi* api, Entity* entity, f32 dt)
{
}
