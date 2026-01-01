#include "internal.h"

extern GameContext game_context;

Parjicle* parjicle_create(vec3 position)
{
    Parjicle* parjicle = st_malloc(sizeof(Parjicle));
    parjicle->position = position;
    parjicle->direction = vec3_create(1, 0, 0);
    parjicle->color = vec3_create(1, 1, 0);
    parjicle->size = 0.1f;
    parjicle->speed = 1.0f;
    parjicle->rotation = 0.0f;
    return parjicle;
}

void parjicle_destroy(Parjicle* parjicle)
{
    st_free(parjicle);
}

void parjicle_update(Parjicle* parjicle, f32 dt)
{
    parjicle->position = vec3_add(parjicle->position, vec3_scale(parjicle->direction, parjicle->speed * dt));
    parjicle->lifetime -= dt;
}

void parjicle_set_flag(Parjicle* parjicle, ParjicleFlagEnum flag, u32 val)
{
    parjicle->flags = (parjicle->flags & ~(1<<flag)) | (val<<flag);
}

bool parjicle_is_flag_set(Parjicle* parjicle, ParjicleFlagEnum flag)
{
    return (parjicle->flags >> flag) & 1;
}
