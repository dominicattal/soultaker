#include "internal.h"

extern GameContext game_context;

void parjicle_init(void)
{
    game_context.parjicles = list_create();
}

Parjicle* parjicle_create(vec3 position)
{
    Parjicle* parjicle = malloc(sizeof(Parjicle));
    return parjicle;
}

void parjicle_destroy(Parjicle* parjicle)
{
    free(parjicle);
}

void parjicle_cleanup(void)
{
    if (game_context.parjicles == NULL)
        return;
    for (i32 i = 0; i < game_context.parjicles->length; i++)
        free(list_get(game_context.parjicles, i));
    list_destroy(game_context.parjicles);
}
