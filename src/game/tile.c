#include "internal.h"
#include "../renderer.h"

extern GameContext game_context;

void tile_init(void)
{
}

Tile* tile_create(vec2 position, u32 minimap_color)
{
    Tile* tile = st_malloc(sizeof(Tile));
    tile->collide = NULL;
    tile->position = position;
    tile->minimap_color = minimap_color;
    tile->tex = texture_get_id("tile_1");
    tile->flags = 0;
    tile_set_flag(tile, TILE_FLAG_ACTIVE, true);
    return tile;
}

void tile_set_flag(Tile* tile, TileFlagEnum flag, bool val)
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
}

