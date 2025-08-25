#include "../../src/api.h"
#include <string.h>

st_export void level_1_generate(LocalMapGenerationSettings* settings)
{
    if (settings->num_rooms_left == 0) {
        if (strcmp(settings->current_branch, "spawn") == 0) {
            settings->current_branch = "enemy";
            settings->num_rooms_left = 10;
        }
        else if (strcmp(settings->current_branch, "enemy") == 0) {
            settings->current_branch = "boss";
            settings->num_rooms_left = 1;
        }
        else
            settings->finished = true;
    }
}

st_export void level_1_spawn_create(GameApi* api)
{
}

st_export void level_1_enemy_1_create(GameApi* api)
{
    i32 id = api->entity_get_id("dummy");
    vec2 pos = api->vec2_create(4, 4);
    api->room_create_entity(pos, id);
    pos.x = 7.5;
    api->room_create_entity(pos, id);
}

st_export void level_1_enemy_2_create(GameApi* api)
{
}

st_export void level_1_boss_create(GameApi* api)
{
}

