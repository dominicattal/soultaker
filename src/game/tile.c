#include "../game.h"
#include "../renderer.h"
#include <string.h>

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
    tile->uid = game_map_uid(tile, GAME_OBJ_TILE);
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
    game_free_uid(tile->uid);
    st_free(tile);
}

void tile_cleanup(void)
{
}

size_t tile_sizeof(void)
{
    Tile tile;
    return sizeof(tile.position)
        +  sizeof(tile.tex)
        +  sizeof(tile.minimap_color)
        +  sizeof(tile.flags)
        +  sizeof(tile.uid);
}

char* tile_write(Tile* tile, char* buffer)
{
    memcpy(buffer, &tile->position, sizeof(tile->position));
    buffer += sizeof(tile->position);
    memcpy(buffer, &tile->tex, sizeof(tile->tex));
    buffer += sizeof(tile->tex);
    memcpy(buffer, &tile->minimap_color, sizeof(tile->minimap_color));
    buffer += sizeof(tile->minimap_color);
    memcpy(buffer, &tile->flags, sizeof(tile->flags));
    buffer += sizeof(tile->flags);
    memcpy(buffer, &tile->uid, sizeof(tile->uid));
    buffer += sizeof(tile->uid);
    return buffer;
}

Tile* tile_read(char* buffer)
{
    Tile* tile = st_calloc(1, sizeof(Tile));
    memcpy(&tile->position, buffer, sizeof(tile->position));
    buffer += sizeof(tile->position);
    memcpy(&tile->tex, buffer, sizeof(tile->tex));
    buffer += sizeof(tile->tex);
    memcpy(&tile->minimap_color, buffer, sizeof(tile->minimap_color));
    buffer += sizeof(tile->minimap_color);
    memcpy(&tile->flags, buffer, sizeof(tile->flags));
    buffer += sizeof(tile->flags);
    memcpy(&tile->uid, buffer, sizeof(tile->uid));
    buffer += sizeof(tile->uid);
    return tile;
}

