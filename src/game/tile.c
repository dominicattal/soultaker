#include "internal.h"
#include "../renderer.h"

extern GameContext game_context;

void tile_init(void)
{
    game_context.tiles = list_create();
    for (i32 i = -100; i <= 100; i++)
        for (i32 j = -100; j <= 100; j++)
            tile_create(vec2_create(i, j));
}

Tile* tile_create(vec2 position)
{
    Tile* tile = malloc(sizeof(Tile));
    tile->position = position;
    tile->tex = ((int)(position.x + position.y) % 2) ? TEX_TILE_1 : TEX_TILE_2;
    list_append(game_context.tiles, tile);
    return tile;
}

void tile_destroy(Tile* tile)
{
    free(tile);
}

void tile_cleanup(void)
{
    if (game_context.tiles == NULL)
        return;
    for (i32 i = 0; i < game_context.tiles->length; i++)
        free(list_get(game_context.tiles, i));
    list_destroy(game_context.tiles);
}

