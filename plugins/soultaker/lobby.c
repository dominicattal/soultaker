#include "../../src/api.h"

#define FLOOR_1     0x2F3C9E
#define FLOOR_2     0xFF006E
#define FLOOR_3     0x9E3D3D
#define WALL        0x44FFA1

static void create_unit(GlobalApi* api, u32 color, vec2 position)
{
    Wall* wall;
    Tile* tile;
    switch (color) {
        case FLOOR_1:
            tile = api->tile_create(position);
            tile->tex = api->texture_get_id("lobby_floor_1");
            break;
        case FLOOR_2:
            tile = api->tile_create(position);
            tile->tex = api->texture_get_id("lobby_floor_2");
            break;
        case FLOOR_3:
            tile = api->tile_create(position);
            tile->tex = api->texture_get_id("lobby_floor_3");
            break;
        case WALL:
            wall = api->wall_create(position, 2.0f);
            wall->side_tex = api->texture_get_id("lobby_wall_side");
            wall->top_tex = api->texture_get_id("lobby_wall_top");
            break;
    }
}

st_export void game_preset_load_lobby(GlobalApi* api)
{
    api->game_set_player_position(api->vec2_create(60, 30));
    Map* map = api->map_load("assets/maps/lobby.png");
    vec2 position;
    for (i32 i = 0; i < map->length; i++) {
        for (i32 j = 0; j < map->width; j++) {
            position = api->vec2_create(j, i);
            create_unit(api, map->data[i * map->width + j], position);
        }
    }
    api->map_free(map);

    position = api->vec2_create(90, 45);
    i32 dummy_id = api->entity_get_id("dummy");
    Entity* entity = api->entity_create(position, dummy_id); 
    entity->health = 1000000;
}
