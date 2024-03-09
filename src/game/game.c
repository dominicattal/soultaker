#include "game.h"

extern TileMap tilemap;

void game_init(void)
{
    tilemap_init();
    player_init();
}

VertexData game_vertex_data(void)
{
    i32 offset = 0;
    VertexData vertex_data;
    vertex_data.data_size = tilemap_data_size();
    vertex_data.data = malloc(vertex_data.data_size);
    tilemap_vertex_data(vertex_data.data, &offset);
    return vertex_data;
}