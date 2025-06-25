#include "../../src/api.h"

__declspec(dllexport)
void game_preset_load_test(GlobalApi* api)
{
    api->entity_create(api->vec3_create(5,0,5), api->entity_map_id("enemy"));
    i32 tile_tex = api->texture_get_id("tile_2");
    for (i32 i = -10; i <= 10; i++) {
        for (i32 j = -10; j <= 10; j++) {
            Tile* tile = api->tile_create(api->vec2_create(i, j));
            tile->tex = tile_tex;
        }
    }
    Wall* wall = api->wall_create(api->vec2_create(-3, -3), 1.5f);
    wall->size = api->vec2_create(0.5, 0.5);
    wall = api->wall_create(api->vec2_create(-1, -1), 1.5f);
    wall->size = api->vec2_create(1, 0);
    api->wall_create(api->vec2_create(-5, -3), 1.5f);
    i32 a = 11;
    for (i32 i = -a; i <= a; i++) {
        api->wall_create(api->vec2_create(i, -a), 1.5f);
        api->wall_create(api->vec2_create(a, i), 1.5f);
        api->wall_create(api->vec2_create(-i-1, a), 1.5f);
        api->wall_create(api->vec2_create(-a, -i-1), 1.5f);
    }
}
