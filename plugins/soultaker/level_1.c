#include "../../src/api.h"
#include <string.h>

typedef struct {
    i32 num_branches;
    i32 global_int;
} LevelData;

st_export void* level_1_init(GameApi* api)
{
    LevelData* data = api->st_malloc(sizeof(LevelData));
    data->num_branches = 0;
    return data;
}

st_export void level_1_cleanup(GameApi* api, void* data)
{
    api->st_free(data);
}

st_export bool level_1_generate(GameApi* api, LocalMapGenerationSettings* settings)
{
    if (strcmp(settings->current_branch, "dead_end") == 0) {
        if (settings->num_rooms_left == 1)
            settings->create_no_path = true;
    }

    if (settings->num_rooms_left != 0)
        return false;

    if (strcmp(settings->current_branch, "main") == 0) {
        if (strcmp(settings->current_room_type, "spawn") == 0) {
            settings->current_room_type = "enemy";
            settings->num_rooms_left = 1;
            return false;
        }
        if (strcmp(settings->current_room_type, "enemy") == 0) {
            settings->current_room_type = "boss";
            settings->num_rooms_left = 1;
            return false;
        }
    }

    return true;
}

st_export bool level_1_branch(GameApi* api, void* data, LocalMapGenerationSettings* settings)
{
    LevelData* gdata = data;
    if (gdata->num_branches == 3)
        return false;
    int roll = rand() % 10;
    if (roll == 0) {
        settings->current_branch = "dead_end";
        settings->current_room_type = "enemy";
        settings->num_rooms_left = 5 + rand() % 3;
        gdata->num_branches++;
        return true;
    }
    return false;
}

st_export void level_1_spawn_create(GameApi* api, LevelData* data)
{
}

st_export void level_1_spawn_exit(GameApi* api, LevelData* data, i32 num_exits)
{
    api->log_write(DEBUG, "%d", num_exits);
    if (num_exits > 0) return;
    Wall* wall = api->room_create_wall(api->vec2_create(5.0, 5.0), 2.0f, 0.75f, 0.75f, 0xFFFF00);
    wall->side_tex = api->texture_get_id("level_1_wall_1_side");
    wall->top_tex = api->texture_get_id("level_1_wall_2_top");
}

typedef struct {
    i32 test;
} TestArgs;

static void test(GameApi* api, Entity* entity, void* args)
{
    api->log_write(DEBUG, "test %f %f %d", entity->position.x, entity->position.z, ((TestArgs*)args)->test);
}

st_export void level_1_enemy_1_create(GameApi* api, LevelData* data)
{
    TestArgs* args = api->st_malloc(sizeof(TestArgs));
    Trigger* trigger;
    i32 id = api->entity_get_id("dummy");
    vec2 pos = api->vec2_create(10, 4);
    api->room_create_entity(pos, id);
    pos = api->vec2_create(12, 4);
    api->room_create_obstacle(pos);
    pos = api->vec2_create(8, 4);
    api->room_create_parstacle(pos);
    args->test = id;
    trigger = api->room_create_trigger(pos, 0.5f, test, NULL, args);
    api->trigger_set_flag(trigger, TRIGGER_FLAG_ONCE, true);
}

st_export void level_1_enemy_2_create(GameApi* api, LevelData* data)
{
}

st_export void dummy_boss_create(GameApi* api, Entity* entity)
{
    entity->size = 1.0f;
    entity->hitbox_radius = 0.5f;
    entity->health = 1;
    entity->max_health = 1;
    api->map_make_boss(entity);
}

st_export void dummy_boss_destroy(GameApi* api, Entity* entity)
{
    api->log_write(DEBUG, "killed boss");
    api->room_set_tilemap_tile(6, 1, 0x00FF00);
    api->room_set_tilemap_tile(7, 1, 0x00FF00);
    api->room_set_tilemap_tile(8, 1, 0x00FF00);
}

static void start_boss(GameApi* api, Entity* entity, void* args)
{
    i32 id = api->entity_get_id("dummy_boss");
    vec2 pos = api->vec2_create(7.5, 3);
    api->room_create_entity(pos, id);
    api->room_set_tilemap_wall(6, 1, 1.0f, 0xFF0000);
    api->room_set_tilemap_wall(7, 1, 1.0f, 0xFF0000);
    api->room_set_tilemap_wall(8, 1, 1.0f, 0xFF0000);
}

st_export void level_1_boss_create(GameApi* api, LevelData* data)
{
    Trigger* trigger;
    vec2 pos = api->vec2_create(7.5, 7.5);
    trigger = api->room_create_trigger(pos, 0.5f, start_boss, NULL, NULL);
    api->trigger_set_flag(trigger, TRIGGER_FLAG_ONCE, true);
}

