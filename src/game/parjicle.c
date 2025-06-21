#include "internal.h"

extern GameContext game_context;

void parjicle_init(void)
{
    game_context.parjicles = list_create();
}

Parjicle* parjicle_create(vec3 position)
{
    Parjicle* parjicle = st_malloc(sizeof(Parjicle));
    parjicle->position = position;
    parjicle->direction = vec3_create(1, 0, 0);
    parjicle->color = vec3_create(1, 0, 1);
    parjicle->size = 1.0f;
    parjicle->speed = 1.0f;
    parjicle->rotation = 0.0f;
    list_append(game_context.parjicles, parjicle);
    return parjicle;
}

void parjicle_destroy(Parjicle* parjicle)
{
    free(parjicle);
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

void parjicle_cleanup(void)
{
    if (game_context.parjicles == NULL)
        return;
    for (i32 i = 0; i < game_context.parjicles->length; i++)
        free(list_get(game_context.parjicles, i));
    list_destroy(game_context.parjicles);
}
