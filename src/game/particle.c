#include "internal.h"

extern GameContext game_context;

void particle_init(void)
{
    game_context.particles = list_create();
    particle_create( vec3_create(0, 0.5, 1));
}

Particle* particle_create(vec3 position)
{
    Particle* particle = st_malloc(sizeof(Particle));
    particle->position = position;
    particle->direction = vec3_create(0, 0, 1);
    particle->size = 0.5f;
    particle->color = vec3_create(1.0f, 1.0f, 1.0f);
    particle->lifetime = 10.0f;
    particle->speed = 1.0f;
    list_append(game_context.particles, particle);
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

void particle_cleanup(void)
{
    if (game_context.particles != NULL) {
        for (i32 i = 0; i < game_context.particles->length; i++)
            particle_destroy(list_get(game_context.particles, i));
        list_destroy(game_context.particles);
    }
}
