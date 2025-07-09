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
    i32 rotate_direction;
    f32 timer;
} HandData;

typedef struct {
    Entity* hand1;
    Entity* hand2;
    f32 timer1;
} AdvisorData;

typedef enum {
    GROWING,
    ATTACK_1,
    ATTACK_2,
    ATTACK_3
} ShaitanStates;

typedef enum {
    FIREBALL,
    FIRESTORM
} ShaitanHandStates;

void firestorm(Projectile* proj, f32 dt)
{
    proj->rotation += 10 * dt;
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
    data->timer += dt;
    if (data->timer > 2.2f) {
        data->rotate_direction = 1 - data->rotate_direction;
        data->timer -= 2.2f;
    }
}

st_export void hand_of_shaitan_create(GlobalApi* api, Entity* entity)
{
    HandData* data = api->st_malloc(sizeof(HandData));
    entity->data = data;
    entity->size = 2;
    entity->health = 2;
    entity->speed = 5;
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

static void attack_1(GlobalApi* api, Entity* entity)
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
    hand = api->entity_create(api->vec3_create(8, 0, 16.5), id);
    data = hand->data;
    data->daddy = advisor;
    advisor_data->hand2 = hand;
    data->rotate_direction = 1;
}

static void attack_2_firestorm(GlobalApi* api, Entity* entity)
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

static void attack_2_fireball(GlobalApi* api, Entity* entity)
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
        attack_1(api, entity);
        entity->state_timer -= 0.5f;
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
        attack_2_firestorm(api, entity);
        entity->state_timer -= 0.5f;
    }
    if (data->timer1 > 2.1f) {
        attack_2_fireball(api, entity);
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
    entity->size = 0.0f;
    entity->hitbox_radius = 0.5f;
    entity->health = 91;
    entity->state = api->entity_get_state_id(entity, "grow");
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
