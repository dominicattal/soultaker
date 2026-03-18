#include "../game.h"

extern GameContext game_context;

Parjicle* parjicle_create(vec3 position)
{
    Parjicle* parjicle = st_malloc(sizeof(Parjicle));
    parjicle->update = NULL;
    parjicle->destroy = NULL;
    parjicle->data = NULL;
    parjicle->position = position;
    parjicle->velocity = vec3_create(0, 0, 0);
    parjicle->acceleration = vec3_create(0, 0, 0);
    parjicle->color = vec3_create(1, 1, 0);
    parjicle->lifetime = 1.0;
    parjicle->size = 0.1f;
    parjicle->rotation = 0.0f;
    return parjicle;
}

void parjicle_destroy(Parjicle* parjicle)
{
    if (parjicle->destroy != NULL)
        parjicle->destroy(parjicle);
    st_free(parjicle);
}

void parjicle_update(Parjicle* parjicle, f32 dt)
{
    parjicle->position = vec3_add(parjicle->position, vec3_scale(parjicle->velocity, dt));
    parjicle->velocity = vec3_add(parjicle->velocity, vec3_scale(parjicle->acceleration, dt));
    parjicle->lifetime -= dt;
    if (parjicle->update != NULL)
        parjicle->update(parjicle, dt);
}

void parjicle_set_flag(Parjicle* parjicle, ParjicleFlagEnum flag, u32 val)
{
    parjicle->flags = (parjicle->flags & ~(1<<flag)) | (val<<flag);
}

bool parjicle_is_flag_set(Parjicle* parjicle, ParjicleFlagEnum flag)
{
    return (parjicle->flags >> flag) & 1;
}
