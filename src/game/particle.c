#include "internal.h"

extern GameContext game_context;

void particle_init(void)
{
    game_context.particles = list_create();
}

Particle* particle_create(vec3 position)
{
    Particle* particle = malloc(sizeof(Particle));
    return particle;
}

void particle_destroy(Particle* particle)
{
    free(particle);
}

void particle_cleanup(void)
{
    if (game_context.particles == NULL)
        return;
    for (i32 i = 0; i < game_context.particles->length; i++)
        free(list_get(game_context.particles, i));
    list_destroy(game_context.particles);
}
