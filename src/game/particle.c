#include "internal.h"

extern GameContext game_context;

Particle* particle_create(vec3 position)
{
    Particle* particle = st_malloc(sizeof(Particle));
    particle->position = position;
    particle->direction = vec3_create(0, 0, 1);
    particle->size = 0.1f;
    particle->color = vec3_create(1.0f, 1.0f, 1.0f);
    particle->lifetime = 10.0f;
    particle->speed = 1.0f;
    return particle;
}

void particle_update(Particle* particle, f32 dt)
{
    particle->position = vec3_add(particle->position, vec3_scale(particle->direction, particle->speed * dt));
    particle->lifetime -= dt;
}

void particle_destroy(Particle* particle)
{
    st_free(particle);
}
