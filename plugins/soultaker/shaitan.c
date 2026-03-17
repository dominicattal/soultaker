#include "../../src/api.h"

void* shaitan_init(void)
{
    return NULL;
}

void shaitan_cleanup(void* data)
{
}

bool shaitan_generate(LocalMapGenerationSettings* settings)
{
    if (settings->num_rooms_left == 0)
        return true;
    return false;
}

bool shaitan_branch(void* data, LocalMapGenerationSettings* settings)
{
    return false;
}

static void create_bars(i32 side_tex, i32 top_tex, vec2 position)
{
    Wall* wall;
    wall = room_create_wall(vec2_create(position.x, position.y+0.5), 1.5f, 1.0f, 0.1f, 0xFF00FF);
    wall->side_tex = side_tex;
    wall->top_tex = top_tex;
}

typedef struct {
    Entity* daddy;
    f32 target_rad;
    f32 target_timer;
    i32 rotate_direction;
    f32 rotate_timer;
    f32 shoot_timer;
    f32 state_timer;
} HandData;

typedef struct {
    Entity* hand1;
    Entity* hand2;
    f32 timer1;
} AdvisorData;

static void firestorm(Projectile* proj, f32 dt)
{
    proj->rotation += 10 * dt;
}

static void firebullet(Projectile* proj, f32 dt)
{
    proj->rotation += 3 * dt;
}

void hand_of_shaitan_update(Entity* entity, f32 dt)
{
    HandData* data = entity->data;
    if (data->daddy == NULL)
        return;
    // instaneous velocity of a point in the motion of a circle 
    // is the orthogonal of its vector to the origin
    vec2 hand_pos = entity->position;
    vec2 daddy_pos = data->daddy->position;
    //f32 rad = vec2_radians(map_orientation());
    vec2 direction = vec2_normalize(vec2_sub(daddy_pos, hand_pos));
    direction.x *= 2 * data->rotate_direction - 1;
    direction.z *= 2 * data->rotate_direction - 1;
    entity->direction = vec2_create(direction.z, -direction.x);
    //entity->direction = vec2_rotate(direction, 0);
    //entity->direction = vec2_rotate(entity->direction, rad);
    data->rotate_timer += dt;
    data->target_rad += (2 * data->rotate_direction - 1) * 1.25 * dt;
    if (data->rotate_timer > 1.7f) {
        data->rotate_direction = 1 - data->rotate_direction;
        data->rotate_timer -= 1.7f;
    }
}

void shaitan_hand_idle(Entity* entity, f32 dt)
{
    HandData* data = entity->data;
    data->state_timer += dt;
    if (data->state_timer > 1) {
        entity_set_state(entity, "attack_1");
        data->state_timer = 0;
    }
}

void shaitan_hand_attack_1(Entity* entity, f32 dt)
{
    HandData* data = entity->data;
    Projectile* proj;
    vec2 direction;
    data->shoot_timer += dt;
    data->state_timer += dt;
    if (data->state_timer > 5) {
        data->state_timer = 0;
        entity_set_state(entity, "idle");
        return;
    }
    if (data->shoot_timer > 0.2) {
        proj = map_create_projectile(entity->position);
        proj->update = firebullet;
        direction = vec2_direction(data->target_rad-PI/3);
        proj->direction = direction;
        proj->speed = 5;
        proj->size = 0.75;
        proj->facing = data->target_rad;
        proj->tex = texture_get_id("shaitan_firebullet");

        proj = map_create_projectile(entity->position);
        proj->update = firebullet;
        direction = vec2_direction(data->target_rad+PI/3);
        proj->direction = direction;
        proj->speed = 5;
        proj->size = 0.75;
        proj->facing = data->target_rad;
        proj->tex = texture_get_id("shaitan_firebullet");

        data->shoot_timer -= 0.2;
    }
}

void shaitan_hand_attack_2(Entity* entity, f32 dt)
{
    HandData* data = entity->data;
    Projectile* proj;
    vec2 direction;
    data->shoot_timer += dt;
    data->state_timer += dt;
    if (data->state_timer > 5) {
        data->state_timer = 0;
        entity_set_state(entity, "idle");
        return;
    }
    if (data->shoot_timer > 0.2) {
        proj = map_create_projectile(entity->position);
        proj->update = firebullet;
        direction = vec2_direction(data->target_rad-PI/3);
        proj->direction = direction;
        proj->speed = 5;
        proj->size = 0.75;
        proj->facing = data->target_rad;
        proj->tex = texture_get_id("shaitan_firebullet");

        proj = map_create_projectile(entity->position);
        proj->update = firebullet;
        direction = vec2_direction(data->target_rad+PI/3);
        proj->direction = direction;
        proj->speed = 5;
        proj->size = 0.75;
        proj->facing = data->target_rad;
        proj->tex = texture_get_id("shaitan_firebullet");

        data->shoot_timer -= 0.2;
    }
}

void hand_of_shaitan_create(Entity* entity)
{
    HandData* data = st_malloc(sizeof(HandData));
    data->daddy = NULL;
    data->rotate_timer = 0;
    data->target_timer = 0;
    data->rotate_direction = 0;
    data->shoot_timer = 0;
    data->state_timer = 0;
    entity->data = data;
    entity->size = 2;
    entity->health = 2;
    entity->max_health = 2;
    entity->speed = 5;
    entity->state = entity_get_state_id(entity, "attack_1");
}

void hand_of_shaitan_destroy(Entity* entity)
{
    HandData* data = entity->data;
    Entity* daddy = data->daddy;
    if (daddy == NULL)
        goto free;
    // daddy is freed before this is called
    //AdvisorData* daddys_data = daddy->data;
    //if (entity == daddys_data->hand1)
    //    daddys_data->hand1 = NULL;
    //else if (entity == daddys_data->hand2)
    //    daddys_data->hand2 = NULL;
free:
    st_free(entity->data);
}

static void spawn_hands(Entity* advisor)
{
    Entity* hand;
    AdvisorData* advisor_data = advisor->data;
    HandData* data;
    i32 id = entity_get_id("hand_of_shaitan");
    //f32 x = advisor->position.x;
    //f32 y = advisor->position.y;
    //hand = entity_create(vec2_create(x+7.5, y), id);
    hand = room_create_entity(vec2_create(8, 20), id);
    data = hand->data;
    data->daddy = advisor;
    advisor_data->hand1 = hand;
    data->rotate_direction = 0;
    data->target_rad = 0.2;
    //hand = entity_create(vec2_create(x-7.5, y), id);
    hand = room_create_entity(vec2_create(23, 20), id);
    data = hand->data;
    data->daddy = advisor;
    advisor_data->hand2 = hand;
    data->rotate_direction = 1;
    data->target_rad = PI-0.2;
}

void shaitan_the_advisor_grow(Entity* entity, f32 dt)
{
    if (entity->size >= 3.0f) {
        entity->state_timer = 0.0f;
        entity_set_state(entity, "attack_1");
        return;
    }
    if (entity->state_timer > 0.2f) {
        entity->state_timer -= 0.2f;
        entity->size += 0.25;
    }
}

static void shaitan_attack_1_firestorm(Entity* entity)
{
    Projectile* proj;
    i32 tex_id = texture_get_id("shaitan_firestorm");
    vec2 direction;
    i32 dir = rand() % 2;
    for (i32 i = 0; i < 5; i++) {
        proj = map_create_projectile(entity->position);
        proj->speed = 4.5f;
        proj->size = 0.75f;
        proj->lifetime = 2.0f;
        proj->tex = tex_id;
        direction = vec2_rotate(vec2_create(dir, 1-dir), PI / 6 * (2-i));
        proj->direction = direction;
        proj->update = firestorm;

        proj = map_create_projectile(entity->position);
        proj->speed = 4.5f;
        proj->size = 0.75f;
        proj->lifetime = 7.3f;
        proj->tex = tex_id;
        direction = vec2_rotate(vec2_create(-dir, -(1-dir)), PI / 6 * (2-i));
        proj->direction = direction;
        proj->update = firestorm;
    }
}

void shaitan_the_advisor_attack_1(Entity* entity, f32 dt)
{
    AdvisorData* data = entity->data;
    if (entity->health <= 99) {
        entity_set_state(entity, "attack_2");
        spawn_hands(entity);
        entity_set_flag(entity, ENTITY_FLAG_INVULNERABLE, 1);
        entity->state_timer = 0;
        entity->frame = 0;
        data->timer1 = 0;
        return;
    }
    if (entity->state_timer > 0.5f) {
        shaitan_attack_1_firestorm(entity);
        entity->state_timer -= 0.5f;
    }
}

static void shaitan_attack_2_firestorm(Entity* entity)
{
    Projectile* proj;
    vec2 position = game_get_nearest_player_position();
    vec2 direction = vec2_sub(position, entity->position);
    i32 tex_id = texture_get_id("shaitan_firestorm");
    proj = map_create_projectile(entity->position);
    proj->speed = 4.5f;
    proj->size = 0.75f;
    proj->lifetime = 2.0f;
    proj->tex = tex_id;
    proj->direction = vec2_normalize(direction);
    proj->update = firestorm;
}

static void shaitan_attack_2_fireball(Entity* entity)
{
    Projectile* proj;
    vec2 direction;
    i32 tex_id = texture_get_id("shaitan_fireball");
    for (i32 i = 0; i < 12; i++) {
        proj = map_create_projectile(entity->position);
        proj->speed = 4.0f;
        proj->size = 0.6f;
        proj->lifetime = 6.0f;
        proj->tex = tex_id;
        direction = vec2_direction(PI / 6 * i);
        proj->direction = direction;
        proj->facing = PI / 6 * i;
    }
}

void shaitan_the_advisor_attack_2(Entity* entity, f32 dt)
{
    AdvisorData* data = entity->data;
    if (data->hand1 == NULL && data->hand2 == NULL) {
        entity->state_timer = 0;
        entity_set_state(entity, "attack_3");
        entity_set_flag(entity, ENTITY_FLAG_INVULNERABLE, 1);
        return;
    }
    entity->frame = entity->state_timer > 0.4f;
    data->timer1 += dt;
    if (entity->state_timer > 0.5f) {
        shaitan_attack_2_firestorm(entity);
        entity->state_timer -= 0.5f;
    }
    if (data->timer1 > 2.1f) {
        shaitan_attack_2_fireball(entity);
        data->timer1 -= 2.1f;
    }
}

void shaitan_the_advisor_attack_3(Entity* entity, f32 dt)
{
}

void shaitan_the_advisor_update(Entity* entity, f32 dt)
{
}

void shaitan_the_advisor_create(Entity* entity)
{
    AdvisorData* data = st_malloc(sizeof(AdvisorData));
    data->hand1 = NULL;
    data->hand2 = NULL;
    entity->data = data;
    entity->size = 0.0f;
    entity->hitbox_radius = 0.5f;
    entity->health = 100;
    entity->max_health = 100;
    entity->state = entity_get_state_id(entity, "grow");
    map_make_boss("Shaitan the Advisor", entity);
}

void shaitan_the_advisor_destroy(Entity* entity)
{
    st_free(entity->data);
}

void shaitan_lava_collide(Entity* entity)
{
    entity_set_flag(entity, ENTITY_FLAG_IN_LAVA, true);
}

void shaitan_lava_create(Tile* tile)
{
    tile_set_flag(tile, TILE_FLAG_ANIMATE_VERTICAL_POS, true);
    tile_set_flag(tile, TILE_FLAG_ANIMATE_HORIZONTAL_NEG, true);
}

void shaitan_spawn_create(void* data)
{
    i32 side_tex, top_tex;
    side_tex = texture_get_id("shaitan_bars_side");
    top_tex = texture_get_id("shaitan_bars_top");
    create_bars(side_tex, top_tex, vec2_create(2, 12));
    create_bars(side_tex, top_tex, vec2_create(6, 10));
    create_bars(side_tex, top_tex, vec2_create(10, 9));
    create_bars(side_tex, top_tex, vec2_create(14, 9));
    create_bars(side_tex, top_tex, vec2_create(15, 9));
    create_bars(side_tex, top_tex, vec2_create(16, 9));
    create_bars(side_tex, top_tex, vec2_create(20, 9));
    create_bars(side_tex, top_tex, vec2_create(24, 10));
    create_bars(side_tex, top_tex, vec2_create(28, 12));

    i32 sta_id = entity_get_id("shaitan_the_advisor");
    room_create_entity(vec2_create(15.5, 20.5), sta_id);
}
