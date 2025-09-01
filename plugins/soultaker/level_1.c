#include "../../src/api.h"
#include <string.h>

typedef struct {
    int num_branches;
} GenerationData;

st_export void* level_1_init(GameApi* api)
{
    GenerationData* data = api->st_malloc(sizeof(GenerationData));
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
            settings->num_rooms_left = 15;
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
    GenerationData* gdata = data;
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

st_export void level_1_spawn_create(GameApi* api)
{
}

st_export void level_1_enemy_1_create(GameApi* api)
{
    i32 id = api->entity_get_id("dummy");
    vec2 pos = api->vec2_create(9, 4);
    api->room_create_entity(pos, id);
}

st_export void level_1_enemy_2_create(GameApi* api)
{
}

st_export void level_1_boss_create(GameApi* api)
{
}

