#include "../../src/api.h"
#include <string.h>

typedef struct {
    i32 num_branches;
    i32 global_int;
    Trigger* boss_trigger;
} LevelData;

void* level_1_init(void)
{
    LevelData* data = st_malloc(sizeof(LevelData));
    data->num_branches = 0;
    return data;
}

void level_1_cleanup(void* data)
{
    st_free(data);
}

bool level_1_generate(LocalMapGenerationSettings* settings)
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
        } else if (strcmp(settings->current_room_type, "enemy") == 0) {
            settings->current_room_type = "boss";
            settings->num_rooms_left = 1;
            return false;
        }
    }

    return true;
}

bool level_1_branch(LocalMapGenerationSettings* settings)
{
    LevelData* gdata = map_get_data();
    if (gdata->num_branches == 6)
        return false;
    int roll = rand() % 10;
    if (roll >= 0) {
        settings->current_branch = "dead_end";
        settings->current_room_type = "enemy";
        settings->num_rooms_left = 5 + rand() % 3;
        settings->num_rooms_left = 100;
        settings->succeed_even_if_no_path = true;
        gdata->num_branches++;
        return true;
    }
    return false;
}

void level_1_spawn_create(void)
{
}

void level_1_spawn_exit(i32 num_exits)
{
    log_write(DEBUG, "%d", num_exits);
    if (num_exits > 0) return;
    Wall* wall = room_create_wall(vec2_create(5.0, 5.0), 2.0f, 0.75f, 0.75f, 0xFFFF00);
    wall->side_tex = texture_get_id("level_1_wall_1_side");
    wall->top_tex = texture_get_id("level_1_wall_2_top");
}

typedef struct {
    i32 test;
} TestData;

static void test(Trigger* trigger, Entity* entity)
{
    TestData* data = trigger->data;
    log_write(DEBUG, "test %f %f %d", entity->position.x, entity->position.z, data->test);
}

void level_1_enemy_1_create(void)
{
    TestData* test_data = st_malloc(sizeof(TestData));
    Trigger* trigger;
    i32 id = entity_get_id("dummy");
    vec2 pos = vec2_create(10, 4);
    room_create_entity(pos, id);
    pos = vec2_create(12, 4);
    room_create_obstacle(pos);
    pos = vec2_create(8, 4);
    room_create_parstacle(pos);
    test_data->test = id;
    trigger = room_create_trigger(pos, 0.5f);
    trigger->enter = test;
    trigger->data = test_data;
    trigger_set_flag(trigger, TRIGGER_FLAG_ONCE, true);
}

void level_1_enemy_2_create(void)
{
}

void dummy_boss_create(Entity* entity)
{
    entity->size = 1.0f;
    entity->hitbox_radius = 0.5f;
    entity->health = 1;
    entity->max_health = 1;
    map_make_boss("dummy boss", entity);
}

void dummy_boss_destroy(Entity* entity)
{
    log_write(DEBUG, "killed boss");
    room_set_tilemap_tile(6, 1, 0x00FF00);
    room_set_tilemap_tile(7, 1, 0x00FF00);
    room_set_tilemap_tile(8, 1, 0x00FF00);
}

static void start_boss(void)
{
    i32 id = entity_get_id("dummy_boss");
    vec2 pos = vec2_create(7.5, 3);
    LevelData* data = map_get_data();
    room_create_entity(pos, id);
    room_set_tilemap_wall(6, 1, 1.0f, 0xFF0000);
    room_set_tilemap_wall(7, 1, 1.0f, 0xFF0000);
    room_set_tilemap_wall(8, 1, 1.0f, 0xFF0000);
    trigger_set_flag(data->boss_trigger, TRIGGER_FLAG_DELETE, true);
    map_set_interactable(NULL, NULL);
}

static void enter_test(Trigger* trigger, Entity* entity)
{
    map_set_interactable("start boss", start_boss);
}

static void stay_test(Trigger* trigger, Entity* entity)
{
}

static void leave_test(Trigger* trigger, Entity* entity)
{
    map_set_interactable(NULL, NULL);
}

void level_1_boss_create(void)
{
    Trigger* trigger;
    LevelData* data = map_get_data();
    vec2 pos = vec2_create(7.5, 7.5);
    trigger = room_create_trigger(pos, 0.5f);
    trigger->enter = enter_test;
    trigger->stay = stay_test;
    trigger->leave = leave_test;
    data->boss_trigger = trigger;
    //trigger_set_flag(trigger, TRIGGER_FLAG_ONCE, true);
}

