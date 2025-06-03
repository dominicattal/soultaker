#include "internal.h"
#include "../renderer.h"

extern GameContext game_context;

void tile_init(void)
{
    log_write(INFO, "Initializing tiles...");
    game_context.tiles = list_create();
    for (i32 i = -10; i <= 10; i++)
        for (i32 j = -10; j <= 10; j++)
            tile_create(vec2_create(i, j));
    log_write(INFO, "Initialized tiles");
}

Tile* tile_create(vec2 position)
{
    Tile* tile = malloc(sizeof(Tile));
    tile->position = position;
    tile->tex = texture_get_id("tile_1");
    list_append(game_context.tiles, tile);
    return tile;
}

void tile_destroy(Tile* tile)
{
    free(tile);
}

void tile_cleanup(void)
{
    log_write(INFO, "Cleaning up tiles...");
    if (game_context.tiles == NULL)
        return;
    for (i32 i = 0; i < game_context.tiles->length; i++)
        free(list_get(game_context.tiles, i));
    list_destroy(game_context.tiles);
    log_write(INFO, "Cleaned up tiles");
}

