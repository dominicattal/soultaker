#include "internal.h"

extern GameContext game_context;

Parstacle* parstacle_create(vec2 position)
{
    Parstacle* parstacle = st_malloc(sizeof(Parstacle));
    parstacle->position = position;
    parstacle->size = 1.5;
    return parstacle;
}

void parstacle_destroy(Parstacle* parstacle)
{
    st_free(parstacle);
}
