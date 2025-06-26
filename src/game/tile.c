#include "internal.h"
#include "../renderer.h"

extern GameContext game_context;

void tile_init(void)
{
    log_write(INFO, "Initializing tiles...");
    game_context.tiles = list_create();
    log_write(INFO, "Initialized tiles");
}

void tile_clear(void)
{
    if (game_context.tiles == NULL)
        return;
    while (game_context.tiles->length > 0)
        tile_destroy(list_remove(game_context.tiles, 0));
}

void tile_lava_collision(Entity* entity)
{
    entity_set_flag(entity, ENTITY_FLAG_IN_LAVA, 1);
}

static void tile_do_nothing(Entity* entity)
{
}

Tile* tile_create(vec2 position)
{
    Tile* tile = st_malloc(sizeof(Tile));
    tile->collide = tile_do_nothing;
    tile->position = position;
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
    log_write(INFO, "Cleaning up tiles...");
    if (game_context.tiles == NULL)
        return;
    for (i32 i = 0; i < game_context.tiles->length; i++)
        tile_destroy(list_get(game_context.tiles, i));
    list_destroy(game_context.tiles);
    log_write(INFO, "Cleaned up tiles");
}

