#include "internal.h"
#include <windows.h>
#include <json.h>

extern GameContext game_context;

typedef void (*InitFuncPtr)(GameApi*);
typedef void (*CleanupFuncPtr)(void);
typedef void (*CreateFuncPtr)(Entity*);
typedef void (*DestroyFuncPtr)(Entity*);
typedef void (*UpdateFuncPtr)(Entity*, f32);
typedef i32  (*TextureFuncPtr)(Entity*);

typedef struct {
    char* handle;
    i32 id;
} State;

typedef struct {
    char* handle;
    InitFuncPtr init;
    CleanupFuncPtr cleanup;
    CreateFuncPtr create;
    DestroyFuncPtr destroy;
    UpdateFuncPtr update;
    TextureFuncPtr texture;
    State* states;
    i32 num_states;
} EntityInfo;

typedef struct {
    HMODULE lib;
    EntityInfo* infos;
    i32 num_entities;
} EntityContext;

static EntityContext entity_context;

static void load_state_info(i32 entity_id, JsonObject* object)
{
    i32 num_states = json_object_length(object);
    State* state_ptr = malloc(num_states * sizeof(State));
    JsonIterator* it = json_iterator_create(object);
    JsonMember* member;
    JsonValue* value;
    const char* state;
    for (i32 i = 0; i < num_states; i++) {
        member = json_iterator_get(it);
        assert(member);
        state = json_member_key(member);
        assert(state);
        value = json_member_value(member);
        assert(value);
        assert(json_get_type(value) == JTYPE_INT);
        state_ptr[i].handle = copy_string(state);
        state_ptr[i].id = json_get_int(value);
        json_iterator_increment(it);
    }
    entity_context.infos[entity_id].num_states = num_states;
    entity_context.infos[entity_id].states = state_ptr;
}

static void load_entity_info(void)
{
    HMODULE lib = entity_context.lib;
    JsonObject* json = json_read("config/entities.json");
    assert(json != NULL);
    JsonIterator* it = json_iterator_create(json);
    assert(it);
    JsonMember* member;
    JsonValue* val_object;
    JsonValue* val_string;
    JsonObject* object;
    const char* string;
    entity_context.num_entities = json_object_length(json);
    entity_context.infos = malloc(entity_context.num_entities * sizeof(EntityInfo));
    for (i32 i = 0; i < entity_context.num_entities; i++) {
        member = json_iterator_get(it);
        assert(member);

        string = json_member_key(member);
        assert(string);
        entity_context.infos[i].handle = copy_string(string);

        val_object = json_member_value(member);
        assert(val_object);
        assert(json_get_type(val_object) == JTYPE_OBJECT);

        object = json_get_object(val_object);
        assert(object);

        val_string = json_get_value(object, "init");
        assert(val_string);
        assert(json_get_type(val_string) == JTYPE_STRING);
        string = json_get_string(val_string);
        assert(string);
        entity_context.infos[i].init = (InitFuncPtr)GetProcAddress(lib, string);
        assert(entity_context.infos[i].init);

        val_string = json_get_value(object, "cleanup");
        assert(val_string);
        assert(json_get_type(val_string) == JTYPE_STRING);
        string = json_get_string(val_string);
        assert(string);
        entity_context.infos[i].cleanup = (CleanupFuncPtr)GetProcAddress(lib, string);
        assert(entity_context.infos[i].cleanup);

        val_string = json_get_value(object, "create");
        assert(val_string);
        assert(json_get_type(val_string) == JTYPE_STRING);
        string = json_get_string(val_string);
        assert(string);
        entity_context.infos[i].create = (CreateFuncPtr)GetProcAddress(lib, string);
        assert(entity_context.infos[i].create);

        val_string = json_get_value(object, "destroy");
        assert(val_string);
        assert(json_get_type(val_string) == JTYPE_STRING);
        string = json_get_string(val_string);
        assert(string);
        entity_context.infos[i].destroy = (DestroyFuncPtr)GetProcAddress(lib, string);
        assert(entity_context.infos[i].destroy);

        val_string = json_get_value(object, "update");
        assert(val_string);
        assert(json_get_type(val_string) == JTYPE_STRING);
        string = json_get_string(val_string);
        assert(string);
        entity_context.infos[i].update = (UpdateFuncPtr)GetProcAddress(lib, string);
        assert(entity_context.infos[i].update);

        val_string = json_get_value(object, "texture");
        assert(val_string);
        assert(json_get_type(val_string) == JTYPE_STRING);
        string = json_get_string(val_string);
        assert(string);
        entity_context.infos[i].texture = (TextureFuncPtr)GetProcAddress(lib, string);
        assert(entity_context.infos[i].texture);

        val_object = json_get_value(object, "states");
        assert(json_get_type(val_object) == JTYPE_OBJECT);
        object = json_get_object(val_object);
        load_state_info(i, object);

        json_iterator_increment(it);
    }

    for (i32 i = 0; i < entity_context.num_entities; i++)
        entity_context.infos[i].init(&game_api);

    json_object_destroy(json);
}

i32 entity_map_id(const char* handle)
{
    int l, r, m, a;
    l = 0;
    r = entity_context.num_entities-1;
    while (l <= r) {
        m = l + (r - l) / 2;
        a = strcmp(handle, entity_context.infos[m].handle);
        if (a > 0)
            l = m + 1;
        else if (a < 0)
            r = m - 1;
        else
            return m;
    }
    return -1;
}

i32 entity_map_state_id(Entity* entity, const char* handle)
{
    return 0;
}

void entity_init(void)
{
    entity_context.lib = LoadLibrary("plugins/soultaker.dll");
    assert(entity_context.lib);
    load_entity_info();

    i32 knight_id = entity_map_id("knight");
    game_context.entities = list_create();
    game_context.player.entity = entity_create(vec3_create(0, 0, 0), knight_id);
    game_context.player.entity->direction = vec3_create(0, 0, 0);
    game_context.player.entity->size = 1.0;
    entity_set_flag(game_context.player.entity, ENTITY_FLAG_FRIENDLY, 1);
    entity_create(vec3_create(5, 0, 0), knight_id);
    entity_create(vec3_create(0, 0, 4), knight_id);
    entity_create(vec3_create(5, 0, 4), knight_id);
}

Entity* entity_create(vec3 position, i32 type)
{
    Entity* entity = malloc(sizeof(Entity));
    entity->position = position;
    entity->prev_position = position;
    entity->direction = vec3_create(0, 0, 0);
    entity->facing = vec2_create(1, 0);
    entity->type = type;
    entity->state_timer = 0;

    entity->haste = 0;
    entity->speed = 3.5;
    entity->size = 1.0f;
    entity->health = 1;
    entity->flags = 0;
    entity->state = 0;
    entity_set_flag(entity, ENTITY_FLAG_UPDATE_FACING, 1);

    entity_context.infos[type].create(entity);
    list_append(game_context.entities, entity);
    return entity;
}

void entity_update(Entity* entity, f32 dt)
{
    entity->prev_position = entity->position;
    entity->position = vec3_add(entity->position, vec3_scale(entity->direction, entity->speed * dt));
    entity->state_timer += dt;
    if (entity_get_flag(entity, ENTITY_FLAG_UPDATE_FACING) && vec3_mag(entity->direction) > 0)
        entity->facing = vec2_create(entity->direction.x, entity->direction.z);

    entity_context.infos[entity->type].update(entity, dt);
}

void entity_set_flag(Entity* entity, EntityFlagEnum flag, u32 val)
{
    entity->flags = (entity->flags & ~(1<<flag)) | (val<<flag);
}

bool entity_get_flag(Entity* entity, EntityFlagEnum flag)
{
    return (entity->flags >> flag) & 1;
}

void entity_set_state(Entity* entity, i32 state)
{
    entity->state = state;
}

i32 entity_get_direction(Entity* entity)
{
    f32 entity_rad = vec2_radians(entity->facing);
    f32 camera_rad = game_context.camera.yaw;
    return get_direction(entity_rad - camera_rad);
}

i32 entity_get_texture(Entity* entity)
{
    return entity_context.infos[entity->type].texture(entity);
}

void entity_destroy(Entity* entity)
{
    entity_context.infos[entity->type].destroy(entity);
    free(entity);
}

void entity_cleanup(void)
{
    if (game_context.entities == NULL)
        return;
    for (i32 i = 0; i < game_context.entities->length; i++)
        free(list_get(game_context.entities, i));
    list_destroy(game_context.entities);
    for (i32 i = 0; i < entity_context.num_entities; i++) {
        entity_context.infos[i].cleanup();
        free(entity_context.infos[i].handle);
        for (i32 j = 0; j < entity_context.infos[i].num_states; j++)
            free(entity_context.infos[i].states[j].handle);
        free(entity_context.infos[i].states);
    }
    free(entity_context.infos);
    FreeLibrary(entity_context.lib);
}
