#include "../tile.h"

void shaitan_lava_create(Tile *tile)
{
    tile->tex = SHAITAN_LAVA_TEX;
    tile->interactable = TRUE;
}

void shaitan_lava_update(Tile *tile, f32 dt)
{
    tile->timer -= dt;
}

void shaitan_lava_interact(Tile *tile, Entity *entity)
{
    entity->ground_level = FALSE;
    entity->position.y = -0.5;
    if (tile->timer < 0) {
        tile->timer = 0.1;
        entity_damage(entity, 0.1);
    }
}