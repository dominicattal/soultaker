#include "../game.h"
#include "../state.h"
#include <string.h>

typedef struct {
    ParticleCreateFuncPtr create;
    ParticleUpdateFuncPtr update;
    ParticleDestroyFuncPtr destroy;
    const char* name;
} ParticleInfo;

typedef struct {
    ParticleInfo* infos;
    i32 num_particles;
} ParticleContext;

extern GameContext game_context;
static ParticleContext particle_context;

static void* load_function(JsonObject* object, const char* key)
{
    JsonValue* val_string = json_object_get_value(object, key);

    if (val_string == NULL)
        return NULL;

    log_assert(json_value_get_type(val_string) == JTYPE_STRING, "");
    
    const char* function_name = json_value_get_string(val_string);
    log_assert(function_name != NULL, "");

    void* fptr = state_load_function(function_name);
    log_assert(fptr != NULL, "Couldnt find function %s", function_name);

    return fptr;
}

void particle_init(void)
{
    JsonObject* json = state_context.config->particles;
    JsonIterator* it = json_iterator_create(json);

    JsonMember* member;
    JsonValue* val_object;
    JsonObject* object;
    const char* string;
    particle_context.num_particles = json_object_length(json);
    particle_context.infos = st_malloc(particle_context.num_particles * sizeof(ParticleInfo));
    for (i32 i = 0; i < particle_context.num_particles; i++) {
        member = json_iterator_get(it);
        log_assert(member != NULL, "");
        string = json_member_get_key(member);
        log_assert(string != NULL, "");
        val_object = json_member_get_value(member);
        log_assert(val_object != NULL, "");
        object = json_value_get_object(val_object);
        log_assert(object != NULL, "");
        particle_context.infos[i].name = string;
        particle_context.infos[i].create = load_function(object, "create");
        particle_context.infos[i].update = load_function(object, "update");
        particle_context.infos[i].destroy = load_function(object, "destroy");
        json_iterator_increment(it);
    }
    json_iterator_destroy(it);
}

i32 particle_get_id(const char* name)
{
    i32 l, r, m, a;
    l = 0;
    r = particle_context.num_particles-1;
    while (l <= r) {
        m = l + (r - l) / 2;
        a = strcmp(name, particle_context.infos[m].name);
        if (a > 0)
            l = m + 1;
        else if (a < 0)
            r = m - 1;
        else
            return m;
    }
    log_write(WARNING, "Could not get id for %s", name);
    return -1;
}

void particle_cleanup(void)
{
    st_free(particle_context.infos);
}

Particle* particle_create_from_struct(Particle particle)
{
    Particle* part = st_malloc(sizeof(Particle));
    part->data = NULL;
    part->position = particle.position;
    part->velocity = particle.velocity;
    part->acceleration = particle.acceleration;
    part->size = particle.size;
    part->color = particle.color;
    part->lifetime = particle.lifetime;
    part->id = particle.id;
    
    if (part->id != -1) {
        ParticleCreateFuncPtr create = particle_context.infos[part->id].create;
        if (create != NULL)
            create(part);
    }

    return part;
}

void particle_update(Particle* particle, f32 dt)
{
    particle->position = vec3_add(particle->position, vec3_scale(particle->velocity, dt));
    particle->velocity = vec3_add(particle->velocity, vec3_scale(particle->acceleration, dt));
    particle->lifetime -= dt;

    if (particle->id != -1) {
        ParticleUpdateFuncPtr update = particle_context.infos[particle->id].update;
        if (update != NULL)
            update(particle, dt);
    }
}

void particle_destroy(Particle* particle)
{
    if (particle->id != -1) {
        ParticleDestroyFuncPtr destroy = particle_context.infos[particle->id].destroy;
        if (destroy != NULL)
            destroy(particle);
    }
    st_free(particle);
}
