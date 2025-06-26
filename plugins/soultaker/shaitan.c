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
            api->tile_set_flag(tile, TILE_FLAG_ANIMATE_HORIZONTAL_POS, 1);
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

__declspec(dllexport)
void game_preset_load_shaitan(GlobalApi* api)
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
}
