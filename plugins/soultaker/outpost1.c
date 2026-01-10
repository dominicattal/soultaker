#include "../../src/api.h"
#include <string.h>

st_export void* outpost1_init(GameApi* api)
{
    return NULL;
}

st_export void outpost1_cleanup(GameApi* api, void* data)
{
}

st_export bool outpost1_generate(GameApi* api, LocalMapGenerationSettings* settings)
{
    if (strcmp(settings->current_room_type, "enemy") == 0) {
        if (settings->num_rooms_left == 1) {
            settings->create_no_path = true;
            return false;
        }
    }

    if (settings->num_rooms_left != 0)
        return false;

    if (strcmp(settings->current_branch, "main") == 0) {
        if (strcmp(settings->current_room_type, "spawn") == 0) {
            settings->current_room_type = "enemy";
            settings->num_rooms_left = 4;
            return false;
        }
    }

    return true;
}

st_export bool outpost1_branch(GameApi* api, LocalMapGenerationSettings* settings)
{
    return false;
}

st_export void outpost1_spawn_create(GameApi* api)
{
}
