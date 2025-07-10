#include "../../src/api.h"

#define LAVA        0xFFAE00
#define FLOOR_1     0x774F00
#define WALL        0x808080
#define HELLSTONE   0xFF8300

static void create_unit(GlobalApi* api, u32 color, vec2 position)
{
    Wall* wall;
    Tile* tile;
    switch (color) {
        case LAVA: 
            tile = api->tile_create(position);
            tile->tex = api->texture_get_id("shaitan_lava");
            tile->collide = api->tile_lava_collision;
            api->tile_set_flag(tile, TILE_FLAG_ANIMATE_VERTICAL_POS, 1);
            api->tile_set_flag(tile, TILE_FLAG_ANIMATE_HORIZONTAL_NEG, 1);
            break;
        case FLOOR_1:
            tile = api->tile_create(position);
            tile->tex = api->texture_get_id("shaitan_floor_1");
            break;
        case WALL:
            wall = api->wall_create(position, 1.5f);
            wall->side_tex = api->texture_get_id("shaitan_wall_side");
            wall->top_tex = api->texture_get_id("shaitan_wall_top");
            break;
        case HELLSTONE:
            tile = api->tile_create(position);
            tile->tex = api->texture_get_id("shaitan_hellstone");
            break;
    }
}

static void create_bars(GlobalApi* api, i32 side_tex, i32 top_tex, vec2 position)
{
    Wall* wall;
    wall = api->wall_create(api->vec2_create(position.x, position.y+0.5), 1.5f);
    wall->side_tex = side_tex;
    wall->top_tex = top_tex;
    wall->size.y = 0.0;
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

void firestorm(Projectile* proj, f32 dt)
{
    proj->rotation += 10 * dt;
}

void firebullet(Projectile* proj, f32 dt)
{
    proj->rotation += 3 * dt;
}

st_export void hand_of_shaitan_update(GlobalApi* api, Entity* entity, f32 dt)
{
    HandData* data = entity->data;
    Entity* daddy = data->daddy;
    // instaneous velocity of a point in the motion of a circle 
    // is the orthogonal of its vector to the origin
    vec2 hand_pos = api->vec2_create(entity->position.x, entity->position.z);
    vec2 daddy_pos = api->vec2_create(daddy->position.x, daddy->position.z);
    vec2 direction = api->vec2_normalize(api->vec2_sub(daddy_pos, hand_pos));
    direction.x *= 2 * data->rotate_direction - 1;
    direction.y *= 2 * data->rotate_direction - 1;
    entity->direction = api->vec3_create(direction.y, 0.0f, -direction.x);
    data->rotate_timer += dt;
    data->target_rad += (2 * data->rotate_direction - 1) * 1.25 * dt;
    if (data->rotate_timer > 1.7f) {
        data->rotate_direction = 1 - data->rotate_direction;
        data->rotate_timer -= 1.7f;
    }
}

st_export void shaitan_hand_idle(GlobalApi* api, Entity* entity, f32 dt)
{
    HandData* data = entity->data;
    data->state_timer += dt;
    if (data->state_timer > 1) {
        api->entity_set_state(entity, "attack_1");
        data->state_timer = 0;
    }
}

st_export void shaitan_hand_attack_1(GlobalApi* api, Entity* entity, f32 dt)
{
    HandData* data = entity->data;
    Projectile* proj;
    vec2 dir2;
    data->shoot_timer += dt;
    data->state_timer += dt;
    if (data->state_timer > 5) {
        data->state_timer = 0;
        api->entity_set_state(entity, "idle");
        return;
    }
    if (data->shoot_timer > 0.2) {
        proj = api->projectile_create(entity->position);
        proj->update = firebullet;
        dir2 = api->vec2_direction(data->target_rad-PI/3);
        proj->direction = api->vec3_create(dir2.x, 0.0, dir2.y);
        proj->speed = 5;
        proj->size = 0.75;
        proj->facing = data->target_rad;
        proj->tex = api->texture_get_id("shaitan_firebullet");

        proj = api->projectile_create(entity->position);
        proj->update = firebullet;
        dir2 = api->vec2_direction(data->target_rad+PI/3);
        proj->direction = api->vec3_create(dir2.x, 0.0, dir2.y);
        proj->speed = 5;
        proj->size = 0.75;
        proj->facing = data->target_rad;
        proj->tex = api->texture_get_id("shaitan_firebullet");

        data->shoot_timer -= 0.2;
    }
}

st_export void hand_of_shaitan_create(GlobalApi* api, Entity* entity)
{
    HandData* data = api->st_malloc(sizeof(HandData));
    data->rotate_timer = 0;
    data->target_timer = 0;
    data->rotate_direction = 0;
    data->shoot_timer = 0;
    data->state_timer = 0;
    entity->data = data;
    entity->size = 2;
    entity->health = 2;
    entity->speed = 5;
    entity->state = api->entity_get_state_id(entity, "attack_1");
}

st_export void hand_of_shaitan_destroy(GlobalApi* api, Entity* entity)
{
    HandData* data = entity->data;
    Entity* daddy = data->daddy;
    AdvisorData* daddys_data = daddy->data;
    if (entity == daddys_data->hand1)
        daddys_data->hand1 = NULL;
    else if (entity == daddys_data->hand2)
        daddys_data->hand2 = NULL;
    api->st_free(entity->data);
}

static void spawn_hands(GlobalApi* api, Entity* advisor)
{
    Entity* hand;
    AdvisorData* advisor_data = advisor->data;
    HandData* data;
    i32 id = api->entity_get_id("hand_of_shaitan");
    hand = api->entity_create(api->vec3_create(23, 0, 16.5), id);
    data = hand->data;
    data->daddy = advisor;
    advisor_data->hand1 = hand;
    data->rotate_direction = 0;
    data->target_rad = 0;
    hand = api->entity_create(api->vec3_create(8, 0, 16.5), id);
    data = hand->data;
    data->daddy = advisor;
    advisor_data->hand2 = hand;
    data->rotate_direction = 1;
    data->target_rad = PI;
}

st_export void shaitan_the_advisor_grow(GlobalApi* api, Entity* entity, f32 dt)
{
    if (entity->size >= 3.0f) {
        entity->state = api->entity_get_state_id(entity, "attack_1");
        entity->state_timer = 0.0f;
        return;
    }
    if (entity->state_timer > 0.2f) {
        entity->state_timer -= 0.2f;
        entity->size += 0.25;
    }
}

static void shaitan_attack_1_firestorm(GlobalApi* api, Entity* entity)
{
    Projectile* proj;
    i32 tex_id = api->texture_get_id("shaitan_firestorm");
    vec2 direction;
    i32 dir = rand() % 2;
    for (i32 i = 0; i < 5; i++) {
        proj = api->projectile_create(entity->position);
        proj->speed = 4.5f;
        proj->size = 0.75f;
        proj->lifetime = 2.0f;
        proj->tex = tex_id;
        direction = api->vec2_rotate(api->vec2_create(dir, 1-dir), PI / 6 * (2-i));
        proj->direction = api->vec3_create(direction.x, 0.0f, direction.y);
        proj->update = firestorm;

        proj = api->projectile_create(entity->position);
        proj->speed = 4.5f;
        proj->size = 0.75f;
        proj->lifetime = 7.3f;
        proj->tex = tex_id;
        direction = api->vec2_rotate(api->vec2_create(-dir, -(1-dir)), PI / 6 * (2-i));
        proj->direction = api->vec3_create(direction.x, 0.0f, direction.y);
        proj->update = firestorm;
    }
}

st_export void shaitan_the_advisor_attack_1(GlobalApi* api, Entity* entity, f32 dt)
{
    AdvisorData* data = entity->data;
    if (entity->health <= 90) {
        entity->state = api->entity_get_state_id(entity, "attack_2");
        spawn_hands(api, entity);
        api->entity_set_flag(entity, ENTITY_FLAG_INVULNERABLE, 1);
        entity->state_timer = 0;
        entity->frame = 0;
        data->timer1 = 0;
        return;
    }
    if (entity->state_timer > 0.5f) {
        shaitan_attack_1_firestorm(api, entity);
        entity->state_timer -= 0.5f;
    }
}

static void shaitan_attack_2_firestorm(GlobalApi* api, Entity* entity)
{
    Projectile* proj;
    vec3 position = api->game_get_nearest_player_position();
    vec3 direction = api->vec3_sub(position, entity->position);
    i32 tex_id = api->texture_get_id("shaitan_firestorm");
    proj = api->projectile_create(entity->position);
    proj->speed = 4.5f;
    proj->size = 0.75f;
    proj->lifetime = 2.0f;
    proj->tex = tex_id;
    proj->direction = api->vec3_normalize(direction);
    proj->update = firestorm;
}

static void shaitan_attack_2_fireball(GlobalApi* api, Entity* entity)
{
    Projectile* proj;
    vec2 direction;
    i32 tex_id = api->texture_get_id("shaitan_fireball");
    for (i32 i = 0; i < 12; i++) {
        proj = api->projectile_create(entity->position);
        proj->speed = 4.0f;
        proj->size = 0.6f;
        proj->lifetime = 6.0f;
        proj->tex = tex_id;
        direction = api->vec2_direction(PI / 6 * i);
        proj->direction = api->vec3_create(direction.x, 0, direction.y);
        proj->facing = PI / 6 * i;
    }
}

st_export void shaitan_the_advisor_attack_2(GlobalApi* api, Entity* entity, f32 dt)
{
    AdvisorData* data = entity->data;
    if (data->hand1 == NULL && data->hand2 == NULL) {
        entity->state_timer = 0;
        entity->state = api->entity_get_state_id(entity, "attack_3");
        api->entity_set_flag(entity, ENTITY_FLAG_INVULNERABLE, 1);
        return;
    }
    entity->frame = entity->state_timer > 0.4f;
    data->timer1 += dt;
    if (entity->state_timer > 0.5f) {
        shaitan_attack_2_firestorm(api, entity);
        entity->state_timer -= 0.5f;
    }
    if (data->timer1 > 2.1f) {
        shaitan_attack_2_fireball(api, entity);
        data->timer1 -= 2.1f;
    }
}

st_export void shaitan_the_advisor_attack_3(GlobalApi* api, Entity* entity, f32 dt)
{
}

st_export void shaitan_the_advisor_update(GlobalApi* api, Entity* entity, f32 dt)
{
}

st_export void shaitan_the_advisor_create(GlobalApi* api, Entity* entity)
{
    AdvisorData* data = api->st_malloc(sizeof(AdvisorData));
    data->hand1 = NULL;
    data->hand2 = NULL;
    entity->data = data;
    entity->size = 3.0f;
    entity->hitbox_radius = 0.5f;
    entity->health = 90;
    entity->state = api->entity_get_state_id(entity, "attack_1");
    api->entity_make_boss(entity);
}

st_export void shaitan_the_advisor_destroy(GlobalApi* api, Entity* entity)
{
    api->st_free(entity->data);
}

st_export void game_preset_load_shaitan(GlobalApi* api)
{
    api->game_set_player_position(api->vec3_create(20, 0, 20));
    Map* map = api->map_load("assets/maps/shaitan.png");
    vec2 position;
    for (i32 i = 0; i < map->length; i++) {
        for (i32 j = 0; j < map->width; j++) {
            position = api->vec2_create(j, i);
            create_unit(api, map->data[i * map->width + j], position);
        }
    }
    api->map_free(map);

    i32 side_tex, top_tex;
    side_tex = api->texture_get_id("shaitan_bars_side");
    top_tex = api->texture_get_id("shaitan_bars_top");
    create_bars(api, side_tex, top_tex, api->vec2_create(14, 27));
    create_bars(api, side_tex, top_tex, api->vec2_create(15, 27));
    create_bars(api, side_tex, top_tex, api->vec2_create(16, 27));
    create_bars(api, side_tex, top_tex, api->vec2_create(10, 27));
    create_bars(api, side_tex, top_tex, api->vec2_create(6, 26));
    create_bars(api, side_tex, top_tex, api->vec2_create(2, 24));
    create_bars(api, side_tex, top_tex, api->vec2_create(20, 27));
    create_bars(api, side_tex, top_tex, api->vec2_create(24, 26));
    create_bars(api, side_tex, top_tex, api->vec2_create(28, 24));

    i32 sta_id = api->entity_get_id("shaitan_the_advisor");
    api->entity_create(api->vec3_create(15.5, 0, 16.5), sta_id);
}
