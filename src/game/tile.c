#include "internal.h"
#include "../renderer.h"

extern GameContext game_context;

void tile_init(void)
{
    game_context.tiles = list_create();
}

void tile_clear(void)
{
    if (game_context.tiles == NULL)
        return;
    while (game_context.tiles->length > 0)
        tile_destroy(list_remove(game_context.tiles, 0));
}

Tile* tile_create(vec2 position, u32 minimap_color)
{
    Tile* tile = st_malloc(sizeof(Tile));
    tile->collide = NULL;
    tile->position = position;
    tile->minimap_color = minimap_color;
    tile->tex = texture_get_id("tile_1");
    tile->flags = 0;
    list_append(game_context.tiles, tile);
    return tile;
}

void tile_set_flag(Tile* tile, TileFlagEnum flag, u32 val)
{
    tile->flags = (tile->flags & ~(1<<flag)) | (val<<flag);
}

bool tile_get_flag(Tile* tile, TileFlagEnum flag)
{
    return (tile->flags >> flag) & 1;
}

void tile_destroy(Tile* tile)
{
    st_free(tile);
}

void tile_cleanup(void)
{
    if (game_context.tiles == NULL)
        return;
    for (i32 i = 0; i < game_context.tiles->length; i++)
        tile_destroy(list_get(game_context.tiles, i));
    list_destroy(game_context.tiles);
}

