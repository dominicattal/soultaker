#include "../../src/api.h"

__declspec(dllexport)
void game_preset_load_shaitan(GlobalApi* api)
{
    i32 tile_tex = api->texture_get_id("shaitan_floor_1");
    for (i32 i = -10; i <= 10; i++) {
        for (i32 j = -10; j <= 10; j++) {
            Tile* tile = api->tile_create(api->vec2_create(i, j));
            tile->tex = tile_tex;
        }
    }
}
