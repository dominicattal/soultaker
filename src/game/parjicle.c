#include "../game.h"
#include "../state.h"
#include <string.h>

typedef struct {
    ParjicleCreateFuncPtr create;
    ParjicleUpdateFuncPtr update;
    ParjicleDestroyFuncPtr destroy;
    const char* name;
} ParjicleInfo;

typedef struct {
    ParjicleInfo* infos;
    i32 num_parjicles;
} ParjicleContext;

extern GameContext game_context;
static ParjicleContext parjicle_context;

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

void parjicle_init(void)
{
    JsonObject* json = state_context.config->parjicles;
    JsonIterator* it = json_iterator_create(json);

    JsonMember* member;
    JsonValue* val_object;
    JsonObject* object;
    const char* string;
    parjicle_context.num_parjicles = json_object_length(json);
    parjicle_context.infos = st_malloc(parjicle_context.num_parjicles * sizeof(ParjicleInfo));
    for (i32 i = 0; i < parjicle_context.num_parjicles; i++) {
        member = json_iterator_get(it);
        log_assert(member != NULL, "");
        string = json_member_get_key(member);
        log_assert(string != NULL, "");
        val_object = json_member_get_value(member);
        log_assert(val_object != NULL, "");
        object = json_value_get_object(val_object);
        log_assert(object != NULL, "");
        parjicle_context.infos[i].name = string;
        parjicle_context.infos[i].create = load_function(object, "create");
        parjicle_context.infos[i].update = load_function(object, "update");
        parjicle_context.infos[i].destroy = load_function(object, "destroy");
        json_iterator_increment(it);
    }
    json_iterator_destroy(it);
}

void parjicle_cleanup(void)
{
    st_free(parjicle_context.infos);
}

i32 parjicle_get_id(const char* name)
{
    i32 l, r, m, a;
    l = 0;
    r = parjicle_context.num_parjicles-1;
    while (l <= r) {
        m = l + (r - l) / 2;
        a = strcmp(name, parjicle_context.infos[m].name);
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

Parjicle* parjicle_create_from_struct(Parjicle parjicle)
{
    Parjicle* parj = st_malloc(sizeof(Parjicle));
    parj->data = NULL;
    parj->position = parjicle.position;
    parj->velocity = parjicle.velocity;
    parj->acceleration = parjicle.acceleration;
    parj->color = parjicle.color;
    parj->lifetime = parjicle.lifetime;
    parj->size = parjicle.size;
    parj->rotation = parjicle.rotation;
    parj->rotate_tex = parjicle.rotate_tex;
    parj->id = parjicle.id;

    if (parj->id != -1) {
        ParjicleCreateFuncPtr create = parjicle_context.infos[parj->id].create;
        if (create != NULL)
            create(parj);
    }

    return parj;
}

void parjicle_destroy(Parjicle* parjicle)
{
    if (parjicle->id != -1) {
        ParjicleDestroyFuncPtr destroy = parjicle_context.infos[parjicle->id].destroy;
        if (destroy != NULL)
            destroy(parjicle);
    }
    st_free(parjicle);
}

void parjicle_update(Parjicle* parjicle, f32 dt)
{
    parjicle->position = vec3_add(parjicle->position, vec3_scale(parjicle->velocity, dt));
    parjicle->velocity = vec3_add(parjicle->velocity, vec3_scale(parjicle->acceleration, dt));
    parjicle->lifetime -= dt;

    if (parjicle->id != -1) {
        ParjicleUpdateFuncPtr update = parjicle_context.infos[parjicle->id].update;
        if (update != NULL)
            update(parjicle, dt);
    }
}
