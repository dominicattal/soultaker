#include "../game.h"

AOE* aoe_create(vec2 position, f32 lifetime)
{
    AOE* aoe = st_malloc(sizeof(AOE));
    aoe->update = NULL;
    aoe->destroy = NULL;
    aoe->data = NULL;
    aoe->position = position;
    aoe->lifetime = 0;
    aoe->damage = 1;
    aoe->timer = 0;
    aoe->cooldown = 0.5;
    aoe->radius = 2;
    aoe->flags = 0;
    return aoe;
}

void aoe_update(AOE* aoe, f32 dt)
{
    aoe->lifetime -= dt;
    if (aoe->timer < 0)
        aoe->timer += aoe->cooldown;
    aoe->timer -= dt;
    if (aoe->update != NULL)
        aoe->update(&game_api, aoe, dt);
}

void aoe_destroy(AOE* aoe)
{
    if (aoe->destroy != NULL)
        aoe->destroy(&game_api, aoe);
    st_free(aoe);
}

void aoe_set_flag(AOE* aoe, AOEFlagEnum flag, bool val)
{
    aoe->flags = (aoe->flags & ~(1<<flag)) | (val<<flag);
}

bool aoe_get_flag(AOE* aoe, AOEFlagEnum flag)
{
    return (aoe->flags >> flag) & 1;
}
