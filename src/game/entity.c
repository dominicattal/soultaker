#include "internal.h"
#include "../state.h"
#include "../api.h"
#include <windows.h>
#include <json.h>

extern GameContext game_context;

typedef void (*InitFuncPtr)(GlobalApi*);
typedef void (*CleanupFuncPtr)(GlobalApi*);
typedef void (*CreateFuncPtr)(GlobalApi*, Entity*);
typedef void (*DestroyFuncPtr)(GlobalApi*, Entity*);
typedef void (*UpdateFuncPtr)(GlobalApi*, Entity*, f32);

typedef struct {
    char* name;
    i32 id;
    i32 num_frames;
    i32* textures;
} EntityState;

typedef struct {
    char* name;
    InitFuncPtr init;
    CleanupFuncPtr cleanup;
    CreateFuncPtr create;
    DestroyFuncPtr destroy;
    UpdateFuncPtr update;
    EntityState* states;
    i32 num_states;
} EntityInfo;

typedef struct {
    EntityInfo* infos;
    i32 num_entities;
} EntityContext;

static EntityContext entity_context;

static void load_state_textures(EntityState* state, JsonObject* object, const char* dir_str, i32 dir_int)
{
    JsonValue* value;
    JsonArray* array;
    const char* string;
    i32 int_val;
    i32 num_frames = state->num_frames;
    value = json_get_value(object, dir_str);
    log_assert(value, "Could not get value for direction %s for state %s", dir_str, state->name);
    log_assert(value, "Textures for direction %s of state %s is not an array", dir_str, state->name);
    array = json_get_array(value);
    int_val = json_array_length(array);
    log_assert(int_val == num_frames, "Number of frames for state %s direction %s does not match the number of frames specified (%d)", state->name, dir_str, num_frames);
    for (i32 j = 0; j < int_val; j++) {
        value = json_array_get(array, j);
        log_assert(value, "Could not get value at index %d in array for state %s direction %s", j, state->name, dir_str);
        log_assert(value, "Value at index %d in array for state %s direction %s is not a string", j, state->name, dir_str);
        string = json_get_string(value);
        log_assert(string, "Could not get string at index %d in array for state %s direction %s", j, state->name, dir_str);
        state->textures[num_frames * dir_int + j] = texture_get_id(string);
    }
}

static void load_state_info(i32 entity_id, JsonObject* object)
{
    JsonValue* value;
    JsonArray* array;
    const char* string;
    i32 int_val;

    value = json_get_value(object, "states");
    log_assert(value, "Could not get states value");
    log_assert(json_get_type(value) == JTYPE_ARRAY, "States array not found");
    array = json_get_array(value);
    log_assert(array, "Could not get states array");
    i32 num_states = json_array_length(array);
    EntityState* state_ptr = st_malloc(num_states * sizeof(EntityState));
    for (i32 i = 0; i < num_states; i++) {
        value = json_array_get(array, i);
        log_assert(value, "Could not get value from array");
        log_assert(json_get_type(value) == JTYPE_OBJECT, "Value is not the right type");
        object = json_get_object(value);
        log_assert(object, "Could not get object from value in array");
        value = json_get_value(object, "name");
        log_assert(value, "Could not get the name of the state");
        log_assert(json_get_type(value) == JTYPE_STRING, "State name is not a string");
        string = json_get_string(value);
        state_ptr[i].name = copy_string(string);
        value = json_get_value(object, "id");
        log_assert(value, "Could not get the id of state %s", string);
        log_assert(json_get_type(value) == JTYPE_INT, "Id of state %s is not an int", string);
        int_val = json_get_int(value);
        log_assert(int_val == i, "Id of state %s does not match its position in array", string);
        state_ptr[i].id = int_val;
        value = json_get_value(object, "frames");
        log_assert(value, "Could not get the number of frames for state %s", string);
        log_assert(json_get_type(value) == JTYPE_INT, "Number of frames for state %s is is not a int", string);
        int_val = json_get_int(value);
        state_ptr[i].num_frames = int_val;
        state_ptr[i].textures = st_malloc(4 * int_val * sizeof(i32));

        load_state_textures(&state_ptr[i], object, "left", LEFT);
        load_state_textures(&state_ptr[i], object, "right", RIGHT);
        load_state_textures(&state_ptr[i], object, "up", UP);
        load_state_textures(&state_ptr[i], object, "down", DOWN);
    }
    entity_context.infos[entity_id].num_states = num_states;
    entity_context.infos[entity_id].states = state_ptr;
}

#define LOAD_ENTITY_FUNCTION(type, object, name, location) \
    val_string = json_get_value(object, name); \
    log_assert(val_string, "Could not get function string %s from object", name); \
    log_assert(json_get_type(val_string) == JTYPE_STRING, "Function string is not the right type");\
    string = json_get_string(val_string); \
    log_assert(string, "Could not get string"); \
    location = (type)GetProcAddress(global_context.lib, string); \
    log_assert(location, "Could not find function %s in library", name);

static void load_entity_info(void)
{
    JsonObject* json = json_read("config/entities.json");
    log_assert(json, "Could not read entity config file");
    JsonIterator* it = json_iterator_create(json);
    log_assert(it, "Could not create iterator for config file");
    JsonMember* member;
    JsonValue* val_object;
    JsonValue* val_string;
    JsonObject* object;
    const char* string;
    entity_context.num_entities = json_object_length(json);
    entity_context.infos = st_malloc(entity_context.num_entities * sizeof(EntityInfo));
    for (i32 i = 0; i < entity_context.num_entities; i++) {
        member = json_iterator_get(it);
        log_assert(member, "Could not get member from entity config file");

        string = json_member_key(member);
        log_assert(string, "Could not get key from entity config file");
        entity_context.infos[i].name = copy_string(string);

        val_object = json_member_value(member);
        log_assert(val_object, "Could not get value from member");
        log_assert(json_get_type(val_object) == JTYPE_OBJECT, "Unrecognized type encountered while reading entity config file");

        object = json_get_object(val_object);
        log_assert(object, "Could not get object from value");

        LOAD_ENTITY_FUNCTION(InitFuncPtr, object, "init", entity_context.infos[i].init);
        LOAD_ENTITY_FUNCTION(CleanupFuncPtr, object, "cleanup", entity_context.infos[i].cleanup); 
        LOAD_ENTITY_FUNCTION(CreateFuncPtr, object, "create", entity_context.infos[i].create);
        LOAD_ENTITY_FUNCTION(DestroyFuncPtr, object, "destroy", entity_context.infos[i].destroy);
        LOAD_ENTITY_FUNCTION(UpdateFuncPtr, object, "update", entity_context.infos[i].update);

        load_state_info(i, object);

        json_iterator_increment(it);
    }

    for (i32 i = 0; i < entity_context.num_entities; i++)
        entity_context.infos[i].init(&global_api);

    json_iterator_destroy(it);
    json_object_destroy(json);
}

i32 entity_map_id(const char* name)
{
    int l, r, m, a;
    l = 0;
    r = entity_context.num_entities-1;
    while (l <= r) {
        m = l + (r - l) / 2;
        a = strcmp(name, entity_context.infos[m].name);
        if (a > 0)
            l = m + 1;
        else if (a < 0)
            r = m - 1;
        else
            return m;
    }
    log_write(FATAL, "Could not map id %s", name);
    return -1;
}

i32 entity_map_state_id(Entity* entity, const char* name)
{
    return 0;
}

void entity_init(void)
{
    log_write(INFO, "Initializing entities...");
    load_entity_info();

    i32 knight_id = entity_map_id("knight");
    game_context.entities = list_create();
    game_context.player.entity = entity_create(vec3_create(0, 0, 0), knight_id);
    game_context.player.entity->direction = vec3_create(0, 0, 0);
    game_context.player.entity->size = 1.0;
    entity_set_flag(game_context.player.entity, ENTITY_FLAG_FRIENDLY, 1);
    entity_create(vec3_create(5,0,5), entity_map_id("enemy"));
    log_write(INFO, "Initialized entities");
}

Entity* entity_create(vec3 position, i32 type)
{
    Entity* entity = st_malloc(sizeof(Entity));
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
    entity->frame = 0;
    entity_set_flag(entity, ENTITY_FLAG_UPDATE_FACING, 1);

    entity_context.infos[type].create(&global_api, entity);
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

    entity_context.infos[entity->type].update(&global_api, entity, dt);
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
    if (state != entity->state) {
        entity->state = state;
        entity->state_timer = 0;
        entity->frame = 0;
    }
}

i32 entity_get_direction(Entity* entity)
{
    f32 entity_rad = vec2_radians(entity->facing);
    f32 camera_rad = game_context.camera.yaw;
    return get_direction(entity_rad - camera_rad);
}

i32 entity_get_texture(Entity* entity)
{
    EntityInfo info = entity_context.infos[entity->type];
    EntityState state = info.states[entity->state];
    i32 dir = get_direction(vec2_radians(entity->facing) - camera_get_yaw());
    i32 num_frames = state.num_frames;
    return state.textures[num_frames * dir + entity->frame];
}

void entity_destroy(Entity* entity)
{
    entity_context.infos[entity->type].destroy(&global_api, entity);
    st_free(entity);
}

void entity_cleanup(void)
{
    log_write(INFO, "Deleting entities...");
    if (game_context.entities != NULL) {
        for (i32 i = 0; i < game_context.entities->length; i++)
            entity_destroy(list_get(game_context.entities, i));
        list_destroy(game_context.entities);
    }
    for (i32 i = 0; i < entity_context.num_entities; i++) {
        entity_context.infos[i].cleanup(&global_api);
        st_free(entity_context.infos[i].name);
        for (i32 j = 0; j < entity_context.infos[i].num_states; j++) {
            st_free(entity_context.infos[i].states[j].name);
            st_free(entity_context.infos[i].states[j].textures);
        }
        st_free(entity_context.infos[i].states);
    }
    st_free(entity_context.infos);
    log_write(INFO, "Deleted entities");
}
