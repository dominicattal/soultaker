#include "internal.h"

extern GameContext game_context;

void particle_init(void)
{
    game_context.particles = list_create();
    for (i32 i = 0; i < 10; i++) {
        Particle* part = particle_create(vec3_create(i, 0.5, 0));
        part->color = vec3_create(0.0f, 1.0f, 1.0f);
        part->size = 0.1f;
    }
}

Particle* particle_create(vec3 position)
{
    Particle* particle = malloc(sizeof(Particle));
    particle->position = position;
    particle->direction = vec3_create(0, 0, 1);
    particle->lifetime = 10.0f;
    list_append(game_context.particles, particle);
    return particle;
}

void particle_update(Particle* particle, f32 dt)
{
    particle->position = vec3_add(particle->position, vec3_scale(particle->direction, dt));
    particle->lifetime -= dt;
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
