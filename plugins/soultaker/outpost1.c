#include "../../src/api.h"
#include <string.h>

typedef struct {
    Entity* boss;
} MapData;

void* outpost1_init(void)
{
    MapData* map_data = st_malloc(sizeof(MapData));
    map_data->boss = NULL;
    return map_data;
}

void outpost1_cleanup(void* data)
{
    st_free(data);
}

bool outpost1_generate(LocalMapGenerationSettings* settings)
{
    if (settings->num_rooms_left != 0)
        return false;

    if (strcmp(settings->current_branch, "main") == 0) {
        if (strcmp(settings->current_room_type, "spawn") == 0) {
            settings->current_room_type = "boss";
            settings->create_no_path = true;
            settings->num_rooms_left = 1;
            return false;
        } else if (strcmp(settings->current_room_type, "enemy") == 0) {
            settings->current_room_type = "boss";
            settings->create_no_path = true;
            settings->num_rooms_left = 1;
            return false;
        }
    }

    return true;
}

bool outpost1_branch(LocalMapGenerationSettings* settings)
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

void outpost1_big_room_create(void)
{
    vec2 position = vec2_create(14, 14);
    i32 id;
    id = entity_get_id("outpost1_knight");
    room_create_entity(position, id);
    //room_create_entity(position, id);
    //room_create_entity(position, id);
    //id = entity_get_id("outpost1_archer");
    //room_create_entity(position, id);
    //room_create_entity(position, id);
    //room_create_entity(position, id);
    //id = entity_get_id("outpost1_mage");
    //room_create_entity(position, id);
    //room_create_entity(position, id);
    //room_create_entity(position, id);
}

void outpost1_boss_room_create(void)
{
    vec2 position = vec2_create(28.5, 12);
    i32 id;
    id = entity_get_id("outpost1_boss");
    room_create_entity(position, id);
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

void outpost1_knight_create(Entity* entity)
{
    KnightData* data = st_malloc(sizeof(KnightData));
    data->spawn_point = entity->position;
    data->wander_cooldown = 0;
    data->shot_cooldown = 0;
    data->rotate_direction = (2*(rand()%2))-1;
    entity->health = entity->max_health = 10;
    entity->size = 1.5f;
    entity->hitbox_radius = 0.7;
    entity->data = data;
    entity->state = entity_get_state_id(entity, "idle");
}

void outpost1_knight_destroy(Entity* entity)
{
    st_free(entity->data);
}

static bool outpost1_player_in_range(Entity* entity, f32 range)
{
    return vec2_mag(vec2_sub(game_get_nearest_player_position(), entity->position)) < range;
}

static bool outpost1_player_out_of_range(Entity* entity, f32 range)
{
    return vec2_mag(vec2_sub(game_get_nearest_player_position(), entity->position)) > range;
}

void outpost1_knight_idle_update(Entity* entity, f32 dt)
{
    vec2 offset, target, distance;
    KnightData* data = entity->data;
    if (outpost1_player_in_range(entity, KNIGHT_IN_RANGE_THRESHOLD)) {
        entity->state = entity_get_state_id(entity, "attack");
        return;
    }
    data->wander_cooldown -= dt;
    if (data->wander_cooldown < 0) {
        offset = vec2_create(randf() * 3, 0.0f);
        offset = vec2_rotate(offset, randf() * 2 * PI);
        target = vec2_add(data->spawn_point, offset);
        distance = vec2_sub(target, entity->position);
        data->wander_timer = vec2_mag(distance) / entity->speed;
        entity->direction = vec2_normalize(distance);
        entity->state = entity_get_state_id(entity, "wander");
    }
}

void outpost1_knight_wander_update(Entity* entity, f32 dt)
{
    KnightData* data = entity->data;
    if (outpost1_player_in_range(entity, KNIGHT_IN_RANGE_THRESHOLD)) {
        entity->state = entity_get_state_id(entity, "attack");
        return;
    }
    data->wander_timer -= dt;
    if (data->wander_timer > 0)
        return;
    entity->direction = vec2_create(0, 0);
    data->wander_cooldown = randf_range(3.0f, 7.0f);
    entity->state = entity_get_state_id(entity, "idle");
}

void outpost1_knight_attack_update(Entity* entity, f32 dt)
{
    KnightData* data = entity->data;
    Projectile* proj;
    vec2 direction, offset, target;
    vec2 player_position = game_get_nearest_player_position();
    f64 distance = vec2_mag(vec2_sub(player_position, entity->position));
    if (outpost1_player_out_of_range(entity, KNIGHT_OUT_OF_RANGE_THRESHOLD)) {
        entity->state = entity_get_state_id(entity, "idle");
        entity->direction = vec2_create(0, 0);
        return;
    }
    direction = vec2_normalize(vec2_sub(player_position, entity->position));
    if (distance > 3.1) {
        entity->direction = direction;
        return;
    }
    if (entity_get_flag(entity, ENTITY_FLAG_HIT_WALL))
        data->rotate_direction = -data->rotate_direction;

    vec2 player_offset = vec2_sub(entity->position, player_position);
    f64 player_offset_rad = vec2_radians(player_offset);
    f64 player_arc_rad = gmodf(player_offset_rad + data->rotate_direction * dt, 2 * PI);
    offset = vec2_direction(player_arc_rad);
    offset = vec2_scale(offset, 3.0f);
    target = vec2_add(player_position, offset);
    entity->direction = vec2_normalize(vec2_sub(target, entity->position));
    data->shot_cooldown -= dt;
    if (entity_get_flag(entity, ENTITY_FLAG_HIT_WALL)) {
        log_write(DEBUG, "%f %f", player_position.x, player_position.z);
        log_write(DEBUG, "%f %f %f %f %i", entity->direction.x, entity->direction.z, entity->position.x, entity->position.z, data->rotate_direction);
    }
    if (data->shot_cooldown < 0) {
        proj = map_create_projectile(entity->position);
        proj->direction = vec2_rotate(direction, randf_range(-0.3, 0.3));
        proj->speed = 6.5f;
        proj->size = 0.5f;
        proj->tex = texture_get_id("bullet");
        proj->facing = vec2_radians(proj->direction);
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

void outpost1_archer_create(Entity* entity)
{
    ArcherData* data = st_malloc(sizeof(ArcherData));
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
    entity->state = entity_get_state_id(entity, "idle");
}

void outpost1_archer_destroy(Entity* entity)
{
    st_free(entity->data);
}

static void reposition_archer(Entity* entity)
{
    ArcherData* data = entity->data;
    vec2 player_position = game_get_nearest_player_position();
    vec2 direction = vec2_sub(entity->position, player_position);
    f32 cur_rad = vec2_radians(direction);
    cur_rad += (2*(rand()%2)-1) * randf_range(0.1, 0.2);
    vec2 offset = vec2_scale(vec2_direction(cur_rad), 6.5f);
    vec2 target = vec2_add(player_position, offset);
    direction = vec2_sub(target, entity->position);
    entity->direction = vec2_normalize(direction);
    data->reposition_timer = vec2_mag(direction) / entity->speed;
}

void outpost1_archer_idle_update(Entity* entity, f32 dt)
{
    vec2 offset, target, distance;
    ArcherData* data = entity->data;
    if (outpost1_player_in_range(entity, ARCHER_IN_RANGE_THRESHOLD)) {
        entity->state = entity_get_state_id(entity, "reposition");
        reposition_archer(entity);
        return;
    }
    data->wander_cooldown -= dt;
    if (data->wander_cooldown < 0) {
        offset = vec2_create(randf() * 3, 0.0f);
        offset = vec2_rotate(offset, randf() * 2 * PI);
        target = vec2_add(data->spawn_point, offset);
        distance = vec2_sub(target, entity->position);
        data->wander_timer = vec2_mag(distance) / entity->speed;
        entity->direction = vec2_normalize(distance);
        entity->state = entity_get_state_id(entity, "wander");
    }
}

void outpost1_archer_wander_update(Entity* entity, f32 dt)
{
    ArcherData* data = entity->data;
    if (outpost1_player_in_range(entity, ARCHER_IN_RANGE_THRESHOLD)) {
        entity->state = entity_get_state_id(entity, "reposition");
        reposition_archer(entity);
        return;
    }
    data->wander_timer -= dt;
    if (data->wander_timer > 0)
        return;
    entity->direction = vec2_create(0, 0);
    data->wander_cooldown = randf_range(3.0f, 7.0f);
    entity->state = entity_get_state_id(entity, "idle");
}

void outpost1_archer_attack_update(Entity* entity, f32 dt)
{
    ArcherData* data = entity->data;
    Projectile* proj;
    vec2 direction, player_position;
    data->attack_timer -= dt;
    if (data->attack_timer < 0) {
        proj = map_create_projectile(entity->position);
        player_position = game_get_nearest_player_position();
        direction = vec2_normalize(vec2_sub(player_position, entity->position));
        proj->direction = direction;
        //proj->direction = vec2_rotate(direction, randf_range(-0.3, 0.3));
        proj->speed = 10.0f;
        proj->size = 0.5f;
        proj->tex = texture_get_id("bullet");
        proj->facing = vec2_radians(proj->direction);
        data->attack_timer += 0.5f;
    }
    data->reposition_timer -= dt;
    if (data->reposition_timer < 0) {
        entity->state = entity_get_state_id(entity, "reposition");
        reposition_archer(entity);
    }
}

void outpost1_archer_reposition_update(Entity* entity, f32 dt)
{
    ArcherData* data = entity->data;
    data->reposition_timer -= dt;
    if (data->reposition_timer < 0) {
        entity->state = entity_get_state_id(entity, "attack");
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

void outpost1_mage_create(Entity* entity)
{
    ArcherData* data = st_malloc(sizeof(ArcherData));
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
    entity->state = entity_get_state_id(entity, "idle");
}

void outpost1_mage_destroy(Entity* entity)
{
    st_free(entity->data);
}

static void reposition_mage(Entity* entity)
{
    ArcherData* data = entity->data;
    vec2 player_position = game_get_nearest_player_position();
    vec2 direction = vec2_sub(entity->position, player_position);
    f32 cur_rad = vec2_radians(direction);
    cur_rad += (2*(rand()%2)-1) * randf_range(0.1, 0.2);
    vec2 offset = vec2_scale(vec2_direction(cur_rad), 6.5f);
    vec2 target = vec2_add(player_position, offset);
    direction = vec2_sub(target, entity->position);
    entity->direction = vec2_normalize(direction);
    data->reposition_timer = vec2_mag(direction) / entity->speed;
}

void outpost1_mage_idle_update(Entity* entity, f32 dt)
{
    vec2 offset, target, distance;
    ArcherData* data = entity->data;
    if (outpost1_player_in_range(entity, ARCHER_IN_RANGE_THRESHOLD)) {
        entity->state = entity_get_state_id(entity, "reposition");
        reposition_mage(entity);
        return;
    }
    data->wander_cooldown -= dt;
    if (data->wander_cooldown < 0) {
        offset = vec2_create(randf() * 3, 0.0f);
        offset = vec2_rotate(offset, randf() * 2 * PI);
        target = vec2_add(data->spawn_point, offset);
        distance = vec2_sub(target, entity->position);
        data->wander_timer = vec2_mag(distance) / entity->speed;
        entity->direction = vec2_normalize(distance);
        entity->state = entity_get_state_id(entity, "wander");
    }
}

void outpost1_mage_wander_update(Entity* entity, f32 dt)
{
    ArcherData* data = entity->data;
    if (outpost1_player_in_range(entity, ARCHER_IN_RANGE_THRESHOLD)) {
        entity->state = entity_get_state_id(entity, "reposition");
        reposition_mage(entity);
        return;
    }
    data->wander_timer -= dt;
    if (data->wander_timer > 0)
        return;
    entity->direction = vec2_create(0, 0);
    data->wander_cooldown = randf_range(3.0f, 7.0f);
    entity->state = entity_get_state_id(entity, "idle");
}

static void mage_projectile_update(Projectile* proj, f32 dt)
{
    vec2 player_position = game_get_nearest_player_position();
    vec2 acceleration = vec2_normalize(vec2_sub(player_position, proj->position));
    vec2 direction = vec2_add(proj->direction, vec2_scale(acceleration, 3 * dt));
    proj->direction = vec2_normalize(direction);
    proj->facing = vec2_radians(proj->direction);
}

void outpost1_mage_attack_update(Entity* entity, f32 dt)
{
    ArcherData* data = entity->data;
    Projectile* proj;
    vec2 direction, player_position;
    data->attack_timer -= dt;
    if (data->attack_timer < 0) {
        proj = map_create_projectile(entity->position);
        player_position = game_get_nearest_player_position();
        direction = vec2_normalize(vec2_sub(player_position, entity->position));
        proj->direction = direction;
        //proj->direction = vec2_rotate(direction, randf_range(-0.3, 0.3));
        proj->speed = 10.0f;
        proj->size = 0.5f;
        proj->tex = texture_get_id("bullet");
        proj->facing = vec2_radians(proj->direction);
        proj->update = mage_projectile_update;
        data->attack_timer += 0.5f;
    }
    data->reposition_timer -= dt;
    if (data->reposition_timer < 0) {
        entity->state = entity_get_state_id(entity, "reposition");
        reposition_mage(entity);
    }
}

void outpost1_mage_reposition_update(Entity* entity, f32 dt)
{
    ArcherData* data = entity->data;
    data->reposition_timer -= dt;
    if (data->reposition_timer < 0) {
        entity->state = entity_get_state_id(entity, "attack");
        data->reposition_timer = 3.0f;
    }
}

// *************************************************************
// Boss
// *************************************************************

static vec2 sword_offsets[] = {
    { .x = 0, .z = 0 },
    { .x = 0.5, .z = 0.5 },
    { .x = 0.5, .z = 0.0 },
    { .x = 0.5, .z = 1.0 },
    { .x = 0.5, .z = 1.5 },
    { .x = 0.5, .z = 2.0 },
    { .x = 1.0, .z = 2.0 },
    { .x = 1.5, .z = 2.0 },
    { .x = 2.0, .z = 2.0 },
    { .x = 2.0, .z = 2.5 },
    { .x = 2.0, .z = 3.0 },
    { .x = 1.5, .z = 3.0 },
    { .x = 1.0, .z = 3.0 },
    { .x = 0.5, .z = 3.0 },
    { .x = 0.0, .z = 3.0 },
    { .x = -0.5, .z = 3.0 },
    { .x = -1.0, .z = 3.0 },
    { .x = -1.5, .z = 3.0 },
    { .x = -2.0, .z = 3.0 },
    { .x = -2.0, .z = 2.5 },
    { .x = -2.0, .z = 2.0 },
    { .x = -1.5, .z = 2.0 },
    { .x = -1.0, .z = 2.0 },
    { .x = -0.5, .z = 2.0 },
    { .x = 0.0, .z = 2.0 },
    { .x = -0.5, .z = 1.5 },
    { .x = -0.5, .z = 1.0 },
    { .x = -0.5, .z = 0.5 },
    { .x = -0.5, .z = 0.0 },
    { .x = 1.0, .z = 3.5 },
    { .x = 1.0, .z = 4.0 },
    { .x = 1.0, .z = 4.5 },
    { .x = 1.0, .z = 5.0 },
    { .x = 1.0, .z = 5.5 },
    { .x = 1.0, .z = 6.0 },
    { .x = 1.0, .z = 6.5 },
    { .x = 1.0, .z = 7.0 },
    { .x = 0.5, .z = 7.5 },
    { .x = 0.0, .z = 8.0 },
    { .x = -0.5, .z = 7.5 },
    { .x = -1.0, .z = 7.0 },
    { .x = -1.0, .z = 6.5 },
    { .x = -1.0, .z = 6.0 },
    { .x = -1.0, .z = 5.5 },
    { .x = -1.0, .z = 5.0 },
    { .x = -1.0, .z = 4.5 },
    { .x = -1.0, .z = 4.0 },
    { .x = -1.0, .z = 3.5 },
};

static void boss_start(Trigger* trigger, Entity* entity)
{
    MapData* map_data = map_get_data();
    Entity* boss = map_data->boss;
    log_write(DEBUG, "%f %f", entity->position.x, entity->position.z);
    log_write(DEBUG, "test");
    if (map_data->boss == NULL)
        log_write(FATAL, "Map boss is null");
    map_make_boss("Asgore", boss);
    entity_set_state(boss, "phase1_attack1");
    Wall* wall;
    wall = room_set_tilemap_wall(26, 54, 2.0f, 0x683434);
    wall->top_tex = texture_get_id("outpost1_wall1_top");
    wall->side_tex = texture_get_id("outpost1_wall1_side");
    wall = room_set_tilemap_wall(27, 54, 2.0f, 0x683434);
    wall->top_tex = texture_get_id("outpost1_wall1_top");
    wall->side_tex = texture_get_id("outpost1_wall1_side");
    wall = room_set_tilemap_wall(28, 54, 2.0f, 0x683435);
    wall->top_tex = texture_get_id("outpost1_wall1_top");
    wall->side_tex = texture_get_id("outpost1_wall1_side");
    wall = room_set_tilemap_wall(29, 54, 2.0f, 0x683434);
    wall->top_tex = texture_get_id("outpost1_wall1_top");
    wall->side_tex = texture_get_id("outpost1_wall1_side");
    wall = room_set_tilemap_wall(30, 54, 2.0f, 0x683434);
    wall->top_tex = texture_get_id("outpost1_wall1_top");
    wall->side_tex = texture_get_id("outpost1_wall1_side");
}

typedef struct {
    f32 shot_timer;
    i32 pattern;
} BossData;

void outpost1_boss_create(Entity* entity)
{
    Trigger* trigger = map_create_trigger(entity->position, 10.0f);
    trigger_set_flag(trigger, TRIGGER_FLAG_ONCE, true);
    trigger_set_flag(trigger, TRIGGER_FLAG_PLAYER, true);
    trigger->enter = boss_start;
    entity->size = 3.0;
    entity->health = entity->max_health = 100;
    BossData* data = entity->data = st_malloc(sizeof(BossData));
    data->shot_timer = 0;
    data->pattern = 0;
    entity_set_flag(entity, ENTITY_FLAG_AUTO_FREE_DATA, true);

    MapData* map_data = map_get_data();
    map_data->boss = entity;
    entity_set_flag(entity, ENTITY_FLAG_INVULNERABLE, true);
}

void outpost1_boss_destroy(Entity* entity)
{
}

static void spawn_sword(vec2 origin, vec2 direction)
{
    for (size_t i = 0; i < sizeof(sword_offsets) / sizeof(vec2); i++) {
        vec2 offset = sword_offsets[i];
        vec2 pos = vec2_add(origin, vec2_rotate(offset, vec2_radians(direction) - PI/2));
        Projectile* proj = room_create_projectile(pos);
        proj->direction = direction;
        proj->size = 0.5;
        proj->speed = 25;
        proj->lifetime = 5.0;
        proj->facing = vec2_radians(direction);
        proj->tex = texture_get_id("bullet");
        projectile_set_flag(proj, PROJECTILE_FLAG_FRIENDLY, true);
    }
}

void outpost1_boss_phase1_attack1_update(Entity* entity, f32 dt)
{
    BossData* data = entity->data;
    vec2 origin, direction;
    f32 center = 28.5;
    data->shot_timer -= dt;
    if (data->shot_timer >= 0)
        return;
    i32 room_width = 56;
    i32 spacing = 3;
    i32 num_swords = 17;
    data->shot_timer += 1;
    for (i32 i = 0; i < num_swords; i++) {
        origin = vec2_create(center + (i-num_swords/2)*spacing, 3);
        direction = vec2_create(0, 1);
        spawn_sword(origin, direction);
        origin = vec2_create(center + (i-num_swords/2)*spacing, room_width-3);
        direction = vec2_create(0, -1);
        spawn_sword(origin, direction);
        origin = vec2_create(3, center + (i-num_swords/2)*spacing);
        direction = vec2_create(1, 0);
        spawn_sword(origin, direction);
        origin = vec2_create(room_width-3, center + (i-num_swords/2)*spacing);
        direction = vec2_create(-1, 0);
        spawn_sword(origin, direction);
    }
}
