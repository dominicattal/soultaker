#include "internal.h"

extern GameContext game_context;

void parstacle_init(void)
{
    game_context.parstacles = list_create();
    parstacle_create(vec2_create(3.0, -4.0));
}

Parstacle* parstacle_create(vec2 position)
{
    Parstacle* parstacle = st_malloc(sizeof(Parstacle));
    parstacle->position = position;
    parstacle->size = 1.5;
    list_append(game_context.parstacles, parstacle);
    return parstacle;
}

void parstacle_destroy(Parstacle* parstacle)
{
    free(parstacle);
}

void parstacle_cleanup(void)
{
    if (game_context.parstacles == NULL)
        return;
    for (i32 i = 0; i < game_context.parstacles->length; i++)
        free(list_get(game_context.parstacles, i));
    list_destroy(game_context.parstacles);
}
