#include "../game.h"

extern GameContext game_context;

Particle* particle_create(vec3 position)
{
    Particle* particle = st_malloc(sizeof(Particle));
    particle->update = NULL;
    particle->destroy = NULL;
    particle->data = NULL;
    particle->position = position;
    particle->velocity = vec3_create(0, 0, 0);
    particle->acceleration = vec3_create(0, 0, 0);
    particle->size = 0.1f;
    particle->color = vec3_create(1.0f, 1.0f, 1.0f);
    particle->lifetime = 10.0f;
    return particle;
}

void particle_update(Particle* particle, f32 dt)
{
    particle->position = vec3_add(particle->position, vec3_scale(particle->velocity, dt));
    particle->velocity = vec3_add(particle->velocity, vec3_scale(particle->acceleration, dt));
    particle->lifetime -= dt;
    if (particle->update != NULL)
        particle->update(&game_api, particle, dt);
}

void particle_destroy(Particle* particle)
{
    if (particle->destroy != NULL)
        particle->destroy(&game_api, particle);
    st_free(particle);
}
