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
    vec2 position = vec2_create(28.5, 14);
    //vec2 position = vec2_create(28.5, 28.5);
    //vec2 position = vec2_create(33.5, 26.5);
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

typedef struct {
    f32 x, z, rotation;
} SwordOffset;

static SwordOffset sword_offsets[] = {
    { 0, 0, 0 },
    { 0.5, 0.5, PI/2 },
    { 0.5, 0.0, 0 },
    { 0.5, 1.0, PI/2 },
    { 0.5, 1.5, PI/2 },
    { 0.5, 2.0, 0 },
    { 1.0, 2.0, 0 },
    { 1.5, 2.0, 0 },
    { 2.0, 2.0, 0 },
    { 2.0, 2.5, PI/2 },
    { 2.0, 3.0, 0 },
    { 1.5, 3.0, 0 },
    { 1.0, 3.0, 0 },
    { 0.5, 3.0, 0 },
    { 0.0, 3.0, 0 },
    { -0.5, 3.0, 0 },
    { -1.0, 3.0, 0 },
    { -1.5, 3.0, 0 },
    { -2.0, 3.0, 0 },
    { -2.0, 2.5, PI/2 },
    { -2.0, 2.0, 0 },
    { -1.5, 2.0, 0 },
    { -1.0, 2.0, 0 },
    { -0.5, 2.0, 0 },
    { 0.0, 2.0, 0 },
    { -0.5, 1.5, PI/2 },
    { -0.5, 1.0, PI/2 },
    { -0.5, 0.5, PI/2 },
    { -0.5, 0.0, 0 },
    { 1.0, 3.5, PI/2 },
    { 1.0, 4.0, PI/2 },
    { 1.0, 4.5, PI/2 },
    { 1.0, 5.0, PI/2 },
    { 1.0, 5.5, PI/2 },
    { 1.0, 6.0, PI/2 },
    { 1.0, 6.5, PI/2 },
    { 1.0, 7.0, PI/2 },
    { 0.5, 7.5, PI/2 },
    { 0.0, 8.0, PI/2 },
    { -0.5, 7.5, PI/2 },
    { -1.0, 7.0, PI/2 },
    { -1.0, 6.5, PI/2 },
    { -1.0, 6.0, PI/2 },
    { -1.0, 5.5, PI/2 },
    { -1.0, 5.0, PI/2 },
    { -1.0, 4.5, PI/2 },
    { -1.0, 4.0, PI/2 },
    { -1.0, 3.5, PI/2 },
};

static f32 boss_room_center = 28.5;
static i32 boss_room_width = 56;
static i32 boss_spacing = 3;
static i32 boss_num_swords = 17;

typedef struct {
    f32 boundary_swords_timer;
    f32 boundary_parj_timer;
    f32 invulnerable_timer;
    f32 chase_timer;
    f32 shot_timer;
    f32 shot_timer2;
    i32 pattern;
    i32 phase_pattern;
    i32 attack;
    i32 wall_idx;
    bool started;
} BossData;

typedef struct {
    f32 delay_timer;
    f32 speed;
} SwordProjData;

static void sword_update(Projectile* proj, f32 dt)
{
    SwordProjData* data = proj->data;
    data->delay_timer -= dt;
    if (data->delay_timer < 0) {
        proj->speed = data->speed;
        proj->update = NULL;
    }
}

static void spawn_sword(vec2 origin, vec2 direction, f32 lifetime, f32 speed)
{
    for (size_t i = 0; i < sizeof(sword_offsets) / sizeof(SwordOffset); i++) {
        SwordOffset sword_offset = sword_offsets[i];
        vec2 offset = vec2_create(sword_offset.x, sword_offset.z);
        vec2 pos = vec2_add(origin, vec2_rotate(offset, vec2_radians(direction) - PI/2));
        Projectile* proj = room_create_projectile(pos);
        proj->direction = direction;
        proj->size = 0.5;
        proj->speed = speed;
        proj->lifetime = lifetime;
        proj->facing = vec2_radians(direction) + sword_offset.rotation;
        if (offset.z <= 3.0)
            proj->tex = texture_get_id("outpost1_sword_handle");
        else
            proj->tex = texture_get_id("outpost1_sword_blade");
        projectile_set_flag(proj, PROJECTILE_FLAG_FRIENDLY, false);
    }
}

static void spawn_sword_with_delay(vec2 origin, vec2 direction, f32 lifetime, f32 speed, f32 delay)
{
    for (size_t i = 0; i < sizeof(sword_offsets) / sizeof(SwordOffset); i++) {
        SwordOffset sword_offset = sword_offsets[i];
        vec2 offset = vec2_create(sword_offset.x, sword_offset.z);
        vec2 pos = vec2_add(origin, vec2_rotate(offset, vec2_radians(direction) - PI/2));
        Projectile* proj = room_create_projectile(pos);
        proj->direction = direction;
        proj->size = 0.5;
        proj->speed = 0;
        proj->lifetime = lifetime + delay;
        proj->update = sword_update;
        SwordProjData* data = st_malloc(sizeof(SwordProjData));
        data->delay_timer = delay;
        data->speed = speed;
        proj->data = data;
        proj->facing = vec2_radians(direction) + sword_offset.rotation;
        if (offset.z <= 3.0)
            proj->tex = texture_get_id("outpost1_sword_handle");
        else
            proj->tex = texture_get_id("outpost1_sword_blade");
        projectile_set_flag(proj, PROJECTILE_FLAG_AUTO_FREE_DATA, true);
        projectile_set_flag(proj, PROJECTILE_FLAG_FRIENDLY, false);
    }
    vec2 cur = vec2_add(origin, vec2_scale(direction, 8));
    f32 distance = speed * lifetime + 8;
    f32 cur_distance = vec2_mag(vec2_sub(origin, cur));
    while (cur_distance < distance) {
        Particle* part = room_create_particle(vec3_create(cur.x, 0.5, cur.z));
        f32 g = lerp(0.0, 1.0, distance, cur_distance);
        part->color = vec3_create(1.0, g, 1.0);
        part->size = 0.1;
        part->lifetime = delay;
        cur = vec2_add(cur, vec2_scale(direction, 0.25));
        cur_distance = vec2_mag(vec2_sub(origin, cur));
    }
}

static void spawn_sword_at_wall(i32 idx, i32 side, f32 lifetime, f32 speed)
{
    vec2 origin, direction;
    f32 rad;
    if (side == UP) {
        origin = vec2_create(boss_room_center + (idx-boss_num_swords/2)*boss_spacing, 3);
        direction = vec2_create(0, 1);
    } else if (side == DOWN) {
        origin = vec2_create(boss_room_center + (idx-boss_num_swords/2)*boss_spacing, boss_room_width-2);
        direction = vec2_create(0, -1);
    } else if (side == RIGHT) {
        origin = vec2_create(3, boss_room_center + (idx-boss_num_swords/2)*boss_spacing);
        direction = vec2_create(1, 0);
    } else if (side == LEFT) {
        origin = vec2_create(boss_room_width-2, boss_room_center + (idx-boss_num_swords/2)*boss_spacing);
        direction = vec2_create(-1, 0);
    }
    spawn_sword(origin, direction, lifetime, speed);
}

static void spawn_sword_at_wall_with_delay(i32 idx, i32 side, f32 lifetime, f32 speed, f32 delay)
{
    vec2 direction, origin;
    if (side == UP) {
        origin = vec2_create(boss_room_center + (idx-boss_num_swords/2)*boss_spacing, 3);
        direction = vec2_create(0, 1);
    } else if (side == DOWN) {
        origin = vec2_create(boss_room_center + (idx-boss_num_swords/2)*boss_spacing, boss_room_width-2);
        direction = vec2_create(0, -1);
    } else if (side == RIGHT) {
        origin = vec2_create(3, boss_room_center + (idx-boss_num_swords/2)*boss_spacing);
        direction = vec2_create(1, 0);
    } else if (side == LEFT) {
        origin = vec2_create(boss_room_width-2, boss_room_center + (idx-boss_num_swords/2)*boss_spacing);
        direction = vec2_create(-1, 0);
    }
    spawn_sword_with_delay(origin, direction, lifetime, speed, delay);
}

static void spawn_sword_at_wall_with_delay_toward_player(i32 idx, i32 side, f32 lifetime, f32 speed, f32 delay)
{
    vec2 origin, direction;
    vec2 player_pos = map_to_room_position(game_get_nearest_player_position());
    f32 rad;
    if (side == UP)
        origin = vec2_create(boss_room_center + (idx-boss_num_swords/2)*boss_spacing, 3);
    else if (side == DOWN)
        origin = vec2_create(boss_room_center + (idx-boss_num_swords/2)*boss_spacing, boss_room_width-2);
    else if (side == RIGHT)
        origin = vec2_create(3, boss_room_center + (idx-boss_num_swords/2)*boss_spacing);
    else if (side == LEFT)
        origin = vec2_create(boss_room_width-2, boss_room_center + (idx-boss_num_swords/2)*boss_spacing);
    direction = vec2_normalize(vec2_sub(player_pos, origin));
    spawn_sword_with_delay(origin, direction, lifetime, speed, delay);
}

static void boss_start(Trigger* trigger, Entity* entity)
{
    MapData* map_data = map_get_data();
    Entity* boss = map_data->boss;
    if (map_data->boss == NULL)
        log_write(FATAL, "Map boss is null");
    BossData* data = boss->data;
    data->started = true;
    data->shot_timer = 2.0;
    data->boundary_swords_timer = 3.0;
    data->phase_pattern = 0;
    //data->invulnerable_timer = 6.0;
    entity_set_state(boss, "phase3");
    boss->health = 0.7 * boss->max_health;
    map_make_boss("Asgore", boss);
    data->attack = 0;
    Wall* wall;
    wall = room_set_tilemap_wall(26, 55, 2.0f, 0x683434);
    wall->top_tex = texture_get_id("outpost1_wall1_top");
    wall->side_tex = texture_get_id("outpost1_wall1_side");
    wall = room_set_tilemap_wall(27, 55, 2.0f, 0x683434);
    wall->top_tex = texture_get_id("outpost1_wall1_top");
    wall->side_tex = texture_get_id("outpost1_wall1_side");
    wall = room_set_tilemap_wall(28, 55, 2.0f, 0x683435);
    wall->top_tex = texture_get_id("outpost1_wall1_top");
    wall->side_tex = texture_get_id("outpost1_wall1_side");
    wall = room_set_tilemap_wall(29, 55, 2.0f, 0x683434);
    wall->top_tex = texture_get_id("outpost1_wall1_top");
    wall->side_tex = texture_get_id("outpost1_wall1_side");
    wall = room_set_tilemap_wall(30, 55, 2.0f, 0x683434);
    wall->top_tex = texture_get_id("outpost1_wall1_top");
    wall->side_tex = texture_get_id("outpost1_wall1_side");

    gui_create_notification("You shouldn't have come here");
}

void outpost1_boss_create(Entity* entity)
{
    Trigger* trigger = map_create_trigger(entity->position, 10.0f);
    trigger_set_flag(trigger, TRIGGER_FLAG_ONCE, true);
    trigger_set_flag(trigger, TRIGGER_FLAG_PLAYER, true);
    trigger->enter = boss_start;
    entity->size = 3.0;
    entity->health = entity->max_health = 100;
    entity->speed = 10.0;
    BossData* data = entity->data = st_calloc(1, sizeof(BossData));
    entity_set_flag(entity, ENTITY_FLAG_AUTO_FREE_DATA, true);
    entity_set_flag(entity, ENTITY_FLAG_INVULNERABLE, true);

    MapData* map_data = map_get_data();
    map_data->boss = entity;
}

void outpost1_boss_destroy(Entity* entity)
{
}

static void spawn_parjicles(void)
{
    vec3 position, direction;
    i32 num_tiles = 31;
    i32 p1 = 13;
    i32 p2 = boss_room_width-12;
    for (i32 dir = 0; dir < 4; dir++) {
        for (i32 i = 0; i < 10; i++) {
            switch (dir) {
                case UP:
                    position = vec3_create(p2, 0.5, p1);
                    direction = vec3_create(-1, 0, 0);
                    break;
                case RIGHT:
                    position = vec3_create(p1, 0.5, p1);
                    direction = vec3_create(0, 0, 1);
                    break;
                case DOWN:
                    position = vec3_create(p1, 0.5, p2);
                    direction = vec3_create(1, 0, 0);
                    break;
                case LEFT:
                    position = vec3_create(p2, 0.5, p2);
                    direction = vec3_create(0, 0, -1);
                    break;
                default:
                    position = vec3_create(0,0,0);
                    break;
            }
            position.x += randf_range(-0.1, 0.1);
            position.y += randf_range(-0.1, 0.1);
            position.z += randf_range(-0.1, 0.1);
            Parjicle* parj = room_create_parjicle(position);
            f32 speed = randf_range(15, 25);
            parj->velocity = vec3_scale(direction, speed);
            f32 hue = randf_range(0, 0.5);
            parj->size = -randf_range(0.1, 0.2);
            parj->color = vec3_create(hue, hue, 1);
            parj->lifetime = num_tiles / speed;
        }
    }
}

void outpost1_boss_update(Entity* entity, f32 dt)
{
    BossData* data = entity->data;
    vec2 origin, direction;
    if (!data->started)
        return;
    data->boundary_swords_timer -= dt;
    if (data->boundary_swords_timer < 0) {
        data->boundary_swords_timer += 0.35/2;
        f32 speed = 50;
        f32 lifetime = (f32)(boss_room_width-4-8)/speed;
        for (i32 i = 0; i < 3; i++) {
            spawn_sword_at_wall(i, UP, lifetime, speed);
            spawn_sword_at_wall(i, LEFT, lifetime, speed);
            spawn_sword_at_wall(boss_num_swords-1-i, RIGHT, lifetime, speed);
            spawn_sword_at_wall(boss_num_swords-1-i, DOWN, lifetime, speed);
        }
    }

    data->boundary_parj_timer -= dt;
    if (data->boundary_parj_timer < 0) {
        spawn_parjicles();
        data->boundary_parj_timer += 0.1;
    }

    if (data->invulnerable_timer >= 0) {
        entity_set_flag(entity, ENTITY_FLAG_INVULNERABLE, true);
        data->invulnerable_timer -= dt;
        if (data->invulnerable_timer < 0)
            entity_set_flag(entity, ENTITY_FLAG_INVULNERABLE, false);
    }
}

void outpost1_boss_phase1_update(Entity* entity, f32 dt)
{
    BossData* data = entity->data;
    vec2 origin, direction;
    data->shot_timer -= dt;
    if (data->attack == 0 && entity->health < 0.9 * entity->max_health) {
        data->phase_pattern = 0;
        data->shot_timer = 1.0;
        data->invulnerable_timer = 6;
        data->attack++;
        entity->health = 0.9 * entity->max_health;
    } else if (data->attack == 1 && entity->health < 0.8 * entity->max_health) {
        data->phase_pattern = 0;
        data->shot_timer = 1.0;
        data->invulnerable_timer = 6;
        data->attack++;
        entity->health = 0.8 * entity->max_health;
    } else if (data->attack == 2 && entity->health < 0.7 * entity->max_health) {
        entity_set_state(entity, "phase2");
        data->chase_timer = 2;
        data->phase_pattern = 0;
        data->shot_timer = 1.0;
        data->invulnerable_timer = 4;
        data->attack = 0;
        entity->health = 0.7 * entity->max_health;
    }

    if (data->shot_timer >= 0)
        return;

    f32 lifetime, speed, delay;
    if (data->phase_pattern < 3) {
        lifetime = 1.8;
        delay = 3.0;
        speed = 15.0;
        for (i32 i = 3; i < boss_num_swords-3; i++)
            spawn_sword_at_wall_with_delay(i, UP, lifetime, speed, delay);
        if (++data->phase_pattern < 3)
            data->shot_timer += 0.7;
        else
            data->shot_timer += 5;
    } else {
        delay = 1.0;
        speed = 15.0;
        lifetime = (f32)(boss_room_width-23)/speed;
        i32 opening = rand() % (boss_num_swords-6-1) + 3;
        for (i32 i = 3; i < boss_num_swords-3; i++)
            if (i != opening && i != opening + 1)
                spawn_sword_at_wall_with_delay(i, UP, lifetime, speed, delay);
        data->shot_timer += 1.5;
        i32 num_left_swords = data->attack;
        i32 num_right_swords = data->attack;
        for (i32 i = 3; i < boss_num_swords-5; i++) {
            f32 ratio = (f32)num_left_swords / (boss_num_swords-5-i);
            if (randf() <= ratio) {
                spawn_sword_at_wall_with_delay(i, LEFT, lifetime, speed, delay);
                num_left_swords--;
            }
            ratio = (f32)num_right_swords / (boss_num_swords-5-i);
            if (randf() <= ratio) {
                spawn_sword_at_wall_with_delay(i, RIGHT, lifetime, speed, delay);
                num_right_swords--;
            }
        }
    }
}

typedef struct {
    vec2 origin;
    f32 initial_angle_offset;
    f32 initial_facing;
} ProjCircleData;

static void sword_circle_proj_update(Projectile* proj, f32 dt)
{
    ProjCircleData* data = proj->data;
    vec2 direction = vec2_sub(proj->position, data->origin);
    proj->direction = vec2_create(-direction.z, direction.x);
    proj->facing = data->initial_facing + vec2_radians(proj->direction) - data->initial_angle_offset + PI/2;
}

static void outpost1_boss_phase2_chase(Entity* entity, f32 dt)
{
    BossData* data = entity->data;
    vec2 origin, direction, offset;
    data->chase_timer -= dt;
    if (data->chase_timer >= 0)
        return;
    vec2 player_pos = game_get_nearest_player_position();
    if (data->phase_pattern == 0) {
        offset = vec2_normalize(vec2_sub(entity->position, player_pos));
        offset = vec2_scale(offset, 5.0);
        offset = vec2_sub(vec2_add(player_pos, offset), entity->position);
        entity->direction = vec2_normalize(offset);
        data->chase_timer = vec2_mag(offset) / entity->speed;
        data->phase_pattern = 1;
    } else {
        f32 arc_length = PI;
        f32 speed = 4.5 + 0.75 * data->attack;
        f32 lifetime = arc_length / speed;
        entity->direction = vec2_create(0, 0);
        origin = entity->position;
        direction = vec2_normalize(vec2_sub(player_pos, origin));
        direction = vec2_rotate(direction, -arc_length/2);
        for (size_t i = 0; i < sizeof(sword_offsets) / sizeof(SwordOffset); i++) {
            SwordOffset sword_offset = sword_offsets[i];
            vec2 offset = vec2_create(sword_offset.x, sword_offset.z);
            vec2 pos_offset = vec2_rotate(offset, vec2_radians(direction) - PI/2);
            vec2 pos = vec2_add(origin, pos_offset);
            Projectile* proj = map_create_projectile(pos);
            proj->direction = direction;
            proj->size = 0.5;
            proj->speed = speed;
            proj->lifetime = lifetime;
            proj->facing = vec2_radians(direction) + sword_offset.rotation;
            proj->update = sword_circle_proj_update;
            proj->data = st_malloc(sizeof(ProjCircleData));
            projectile_set_flag(proj, PROJECTILE_FLAG_AUTO_FREE_DATA, true);
            if (offset.z <= 3.0)
                proj->tex = texture_get_id("outpost1_sword_handle");
            else
                proj->tex = texture_get_id("outpost1_sword_blade");
            *(ProjCircleData*)proj->data = (ProjCircleData) { 
                origin, 
                vec2_radians(pos_offset),
                proj->facing
            };
            projectile_set_flag(proj, PROJECTILE_FLAG_FRIENDLY, false);
        }
        data->phase_pattern = 0;
        data->chase_timer = maxf(0.5, lifetime + 0.2);
    }
}

void outpost1_boss_phase2_update(Entity* entity, f32 dt)
{
    BossData* data = entity->data;
    if (data->attack == 0 && entity->health < 0.6 * entity->max_health) {
        data->shot_timer = 1.0;
        entity->speed = 11.0f;
        data->attack++;
    } else if (data->attack == 1 && entity->health < 0.5 * entity->max_health) {
        data->shot_timer = 1.0;
        entity->speed = 12.0f;
        data->attack++;
    } else if (data->attack == 2 && entity->health < 0.4 * entity->max_health) {
        entity_set_state(entity, "phase3");
        data->phase_pattern = 0;
        data->attack = 0;
        data->shot_timer = 2.0;
        data->invulnerable_timer = 4;
        entity->speed = 10.0f;
        entity->health = 0.4 * entity->max_health;
    }
    data->shot_timer -= dt;
    outpost1_boss_phase2_chase(entity, dt);
    if (data->shot_timer > 0)
        return;
    data->shot_timer += 1.0 - 0.21 * data->attack;
    i32 sword_idx = rand() % (boss_num_swords - 6) + 3;
    f32 speed = 15.0;
    f32 delay = 1.0;
    f32 lifetime = (f32)(boss_room_width-23)/speed;
    spawn_sword_at_wall_with_delay(sword_idx, data->wall_idx, lifetime, speed, delay);
    data->wall_idx = (data->wall_idx+1) % 4;
}

void outpost1_boss_phase3_update(Entity* entity, f32 dt)
{
    BossData* data = entity->data;
    vec2 origin = room_to_map_position(vec2_create(28.5, 28.5));
    vec2 offset = vec2_sub(entity->position, origin);
    if (data->phase_pattern == 0) {
        f32 radius = 14.0f;
        f32 rad = (offset.x != 0) ? vec2_radians(offset) : 0;
        vec2 target = vec2_add(origin, vec2_scale(vec2_direction(rad), radius));
        vec2 direction = vec2_sub(target, entity->position);
        data->chase_timer = vec2_mag(direction) / entity->speed;
        entity->direction = vec2_normalize(direction);
        data->phase_pattern = 1;
    } else if (data->phase_pattern == 1) {
        data->chase_timer -= dt;
        if (data->chase_timer < 0) {
            entity->direction = vec2_create(0, 0);
            data->phase_pattern = 2;
        }
    } else {
        vec2 velocity = vec2_normalize(vec2_rotate(offset, -PI / 2));
        entity->direction = velocity;
    }

    data->shot_timer -= dt;
    if (data->shot_timer < 0) {
        data->shot_timer += 1.5;
        f32 delay = 1.0;
        f32 speed = 15.0;
        f32 lifetime = (f32)(boss_room_width-23)/speed;
        i32 num_swords[4] = {1,1,1,1};
        for (i32 dir = 0; dir < 4; dir++) {
            for (i32 i = 3; i < boss_num_swords-5; i++) {
                f32 ratio = (f32)num_swords[dir] / (boss_num_swords-5-i);
                if (randf() <= ratio) {
                    spawn_sword_at_wall_with_delay(i, dir, lifetime, speed, delay);
                    num_swords[dir]--;
                }
            }
        }
    }

    vec2 player_pos = game_get_nearest_player_position();
    data->shot_timer2 -= dt;
    if (data->shot_timer2 < 0) {
        data->shot_timer2 += 1.0;
        f32 delay = 1.0;
        f32 speed = 15.0;
        f32 lifetime = 10.0;
        i32 wall_idx = rand() % 4;
        i32 sword_idx = rand() % boss_num_swords;
        spawn_sword_at_wall_with_delay_toward_player(sword_idx, wall_idx, lifetime, speed, delay);
    }
}
