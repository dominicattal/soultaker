#include "internal.h"

extern GameContext game_context;

void parstacle_init(void)
{
    game_context.parstacles = list_create();
}

void parstacle_clear(void)
{
    if (game_context.parstacles == NULL)
        return;
    while (game_context.parstacles->length > 0)
        parstacle_destroy(list_remove(game_context.parstacles, 0));
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
    st_free(parstacle);
}

void parstacle_cleanup(void)
{
    if (game_context.parstacles == NULL)
        return;
    for (i32 i = 0; i < game_context.parstacles->length; i++)
        parstacle_destroy(list_get(game_context.parstacles, i));
    list_destroy(game_context.parstacles);
}
