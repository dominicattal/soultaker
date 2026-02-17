#include "../game.h"

AOE* aoe_create(vec2 position, f32 lifetime)
{
    AOE* aoe = st_malloc(sizeof(AOE));
    aoe->update = NULL;
    aoe->collision = NULL;
    aoe->destroy = NULL;
    aoe->data = NULL;
    aoe->position = position;
    aoe->lifetime = lifetime;
    aoe->timer = 0;
    aoe->flags = 0;
    return aoe;
}

void aoe_update(AOE* aoe, f32 dt)
{
    aoe->lifetime -= dt;
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
