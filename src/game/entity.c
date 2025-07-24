#include "internal.h"
#include "../state.h"
#include "../api.h"
#include <windows.h>
#include <json.h>
#include <math.h>

extern GameContext game_context;

typedef void (*CreateFuncPtr)(GlobalApi*, Entity*);
typedef void (*DestroyFuncPtr)(GlobalApi*, Entity*);
typedef void (*UpdateFuncPtr)(GlobalApi*, Entity*, f32);

typedef struct {
    char* name;
    UpdateFuncPtr update;
    i32 num_frames;
    f32* frame_lengths;
    i32* frames;
} EntityState;

typedef struct {
    char* name;
    CreateFuncPtr create;
    DestroyFuncPtr destroy;
    UpdateFuncPtr update;
    EntityState* states;
    i32 num_states;
    bool bidirectional;
} EntityInfo;

typedef struct {
    EntityInfo* infos;
    i32 num_entities;
} EntityContext;

static EntityContext entity_context;

static void default_update_function(GlobalApi*, Entity*, f32) {}
static void default_create_function(GlobalApi*, Entity*) {}
static void default_destroy_function(GlobalApi*, Entity*) {}

static CreateFuncPtr load_create_function(JsonObject* object, i32 entity_id)
{
    JsonValue* val_string = json_get_value(object, "create");
    const char* name = entity_context.infos[entity_id].name;
    if (val_string == NULL)
        return default_create_function;
    log_assert(json_get_type(val_string) == JTYPE_STRING, "Create function for %s is not a string", name);
    const char* function_name = json_get_string(val_string);
    log_assert(function_name, "Failed to get create function name for %s from json value", name);
    CreateFuncPtr fptr = state_load_function(function_name);
    log_assert(fptr, "Failed to find function %s for entity %s in library", function_name, name);
    return fptr;
}

static DestroyFuncPtr load_destroy_function(JsonObject* object, i32 entity_id)
{
    JsonValue* val_string = json_get_value(object, "destroy");
    const char* name = entity_context.infos[entity_id].name;
    if (val_string == NULL)
        return default_destroy_function;
    log_assert(json_get_type(val_string) == JTYPE_STRING, "Destroy function for %s is not a string", name);
    const char* function_name = json_get_string(val_string);
    log_assert(function_name, "Failed to get destroy function name for %s from json value", name);
    DestroyFuncPtr fptr = state_load_function(function_name);
    log_assert(fptr, "Failed to find function %s for entity %s in library", function_name, name);
    return fptr;
}

static UpdateFuncPtr load_update_function(JsonObject* object, i32 entity_id)
{
    JsonValue* val_string = json_get_value(object, "update");
    const char* name = entity_context.infos[entity_id].name;
    if (val_string == NULL)
        return default_update_function;
    log_assert(json_get_type(val_string) == JTYPE_STRING, "Update function for %s is not a string", name);
    const char* function_name = json_get_string(val_string);
    log_assert(function_name, "Failed to get update function name for %s from json value", name);
    UpdateFuncPtr fptr = state_load_function(function_name);
    log_assert(fptr, "Failed to find function %s for entity %s in library", function_name, name);
    return fptr;
}

static void load_state_frame_lengths(EntityState* state, JsonObject* object)
{
    JsonValue* value;
    JsonArray* array;
    i32 int_val, i;
    f32 flt_val;
    value = json_get_value(object, "timers");
    if (value == NULL) {
        for (i = 0; i < state->num_frames; i++)
            state->frame_lengths[i] = 0.5;
        return;
    }
    log_assert(json_get_type(value) == JTYPE_ARRAY, "Timers are not in an array");
    array = json_get_array(value);
    int_val  = json_array_length(array);
    log_assert(int_val == state->num_frames, "Number of frames does not match number of frame timers");
    for (i = 0; i < state->num_frames; i++) {
        value = json_array_get(array, i);
        log_assert(value, "Could not get float value");
        log_assert(json_get_type(value) == JTYPE_FLOAT, "Not a float");
        flt_val = json_get_float(value);
        state->frame_lengths[i] = flt_val;
    }
}

static void load_state_frames(EntityState* state, JsonObject* object, const char* dir_str, i32 dir_int)
{
    JsonValue* value;
    JsonArray* array;
    const char* string;
    i32 int_val;
    i32 num_frames = state->num_frames;
    value = json_get_value(object, dir_str);
    log_assert(value, "Could not get value for direction %s for state %s", dir_str, state->name);
    log_assert(json_get_type(value) == JTYPE_ARRAY, "Textures for direction %s of state %s is not an array", dir_str, state->name);
    array = json_get_array(value);
    int_val = json_array_length(array);
    log_assert(int_val == num_frames, "Number of frames for state %s direction %s does not match the number of frames specified (%d)", state->name, dir_str, num_frames);
    for (i32 j = 0; j < num_frames; j++) {
        value = json_array_get(array, j);
        log_assert(value, "Could not get value at index %d in array for state %s direction %s", j, state->name, dir_str);
        log_assert(json_get_type(value) == JTYPE_STRING, "Value at index %d in array for state %s direction %s is not a string", j, state->name, dir_str);
        string = json_get_string(value);
        log_assert(string, "Could not get string at index %d in array for state %s direction %s", j, state->name, dir_str);
        state->frames[num_frames * dir_int + j] = texture_get_id(string);
    }
}

static void load_state_info(i32 entity_id, JsonObject* object)
{
    JsonValue* value;
    JsonObject* obj_states;
    JsonIterator* it;
    JsonMember* member;
    JsonType type;
    const char* name;
    i32 num_frames;
    i32 bidirectional;
    
    bidirectional = 0;
    value = json_get_value(object, "bidirectional");
    if (value != NULL)  {
        type = json_get_type(value);
        if (type == JTYPE_TRUE)
            bidirectional = 1;
    }

    value = json_get_value(object, "states");
    log_assert(value, "Could not get states value");
    log_assert(json_get_type(value) == JTYPE_OBJECT, "States object not found");
    obj_states = json_get_object(value);
    log_assert(obj_states, "Could not get states object");
    it = json_iterator_create(obj_states);
    i32 num_states = json_object_length(obj_states);
    EntityState* state_ptr = st_malloc(num_states * sizeof(EntityState));
    for (i32 i = 0; i < num_states; i++) {
        member = json_iterator_get(it);
        name = json_member_key(member);
        value = json_member_value(member);
        log_assert(value, "Could not get value from member");
        log_assert(json_get_type(value) == JTYPE_OBJECT, "Value is not the right type");
        object = json_get_object(value);
        log_assert(object, "Could not get object from value in array");

        value = json_get_value(object, "frames");
        log_assert(value, "Could not get the number of frames for state %s", name);
        log_assert(json_get_type(value) == JTYPE_INT, "Number of frames for state %s is is not a int", name);
        num_frames = json_get_int(value);

        state_ptr[i].name = string_copy(name);
        state_ptr[i].num_frames = num_frames;
        state_ptr[i].frames = st_malloc(4 * num_frames * sizeof(i32));
        state_ptr[i].frame_lengths = st_malloc(num_frames * sizeof(f32));
        state_ptr[i].update = load_update_function(object, i);

        load_state_frame_lengths(&state_ptr[i], object);
        load_state_frames(&state_ptr[i], object, "left", LEFT);
        load_state_frames(&state_ptr[i], object, "right", RIGHT);
        if (!bidirectional) {
            load_state_frames(&state_ptr[i], object, "up", UP);
            load_state_frames(&state_ptr[i], object, "down", DOWN);
        }

        json_iterator_increment(it);
    }

    json_iterator_destroy(it);

    entity_context.infos[entity_id].num_states = num_states;
    entity_context.infos[entity_id].states = state_ptr;
    entity_context.infos[entity_id].bidirectional = bidirectional;
}

static void load_entity_info(void)
{
    JsonObject* json = json_read("config/entities.json");
    log_assert(json, "Could not read entity config file");
    JsonIterator* it = json_iterator_create(json);
    log_assert(it, "Could not create iterator for config file");
    JsonMember* member;
    JsonValue* val_object;
    JsonObject* object;
    const char* string;
    entity_context.num_entities = json_object_length(json);
    entity_context.infos = st_malloc(entity_context.num_entities * sizeof(EntityInfo));
    for (i32 i = 0; i < entity_context.num_entities; i++) {
        member = json_iterator_get(it);
        log_assert(member, "Could not get member from entity config file");

        string = json_member_key(member);
        log_assert(string, "Could not get key from entity config file");
        entity_context.infos[i].name = string_copy(string);

        val_object = json_member_value(member);
        log_assert(val_object, "Could not get value from member");
        log_assert(json_get_type(val_object) == JTYPE_OBJECT, "Unrecognized type encountered while reading entity config file");

        object = json_get_object(val_object);
        log_assert(object, "Could not get object from value");

        entity_context.infos[i].create = load_create_function(object, i);
        entity_context.infos[i].destroy = load_destroy_function(object, i);
        entity_context.infos[i].update = load_update_function(object, i);
        load_state_info(i, object);

        json_iterator_increment(it);
    }

    json_iterator_destroy(it);
    json_object_destroy(json);
}

i32 entity_get_id(const char* name)
{
    i32 l, r, m, a;
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
    log_write(WARNING, "Could not get id for %s", name);
    return -1;
}

i32 entity_get_state_id(Entity* entity, const char* name)
{
    i32 l, r, m, a;
    l = 0;
    r = entity_context.infos[entity->id].num_states-1;
    while (l <= r) {
        m = l + (r - l) / 2;
        a = strcmp(name, entity_context.infos[entity->id].states[m].name);
        if (a > 0)
            l = m + 1;
        else if (a < 0)
            r = m - 1;
        else
            return m;
    }
    log_write(FATAL, "Could not get state id for %s", name);
    return -1;
}

void entity_set_state(Entity* entity, const char* name)
{
    entity->frame = 0;
    entity->state = entity_get_state_id(entity, name);
}

void entity_init(void)
{
    game_context.entities = list_create();
    game_context.bosses = list_create();

    load_entity_info();
}

void entity_clear(void)
{
    if (game_context.entities == NULL)
        return;
    while (game_context.entities->length > 0)
        entity_destroy(list_remove(game_context.entities, 0));
    game_context.player.entity = NULL;
}

Entity* entity_create(vec2 position, i32 id)
{
    Entity* entity = st_malloc(sizeof(Entity));
    entity->position = position;
    entity->prev_position = position;
    entity->direction = vec2_create(0, 0);
    entity->elevation = 0;
    entity->facing = vec2_create(1, 0);
    entity->id = id;
    entity->state_timer = 0;
    entity->frame_timer = 0;
    entity->tile_timer = 0;
    entity->frame_speed = 1;

    entity->health = 1;
    entity->speed = 7.0f;
    entity->size = 1.0f;
    entity->hitbox_radius = 0.5f;
    entity->flags = 0;
    entity->state = 0;
    entity->frame = 0;
    entity_set_flag(entity, ENTITY_FLAG_UPDATE_FACING, 1);

    entity_context.infos[id].create(&global_api, entity);
    list_append(game_context.entities, entity);
    return entity;
}

static void handle_lava(Entity* entity, f32 dt)
{
    if (entity_get_flag(entity, ENTITY_FLAG_IN_LAVA)) {
        entity->tile_timer += dt;
        entity->elevation = -0.2;
        if (entity->tile_timer > 0.5) {
            entity->tile_timer -= 0.5;
            //entity->health -= 1;
        }
        entity_set_flag(entity, ENTITY_FLAG_IN_LAVA, 0);
    } else {
        entity->elevation = 0;
        entity->tile_timer = 0;
    }
}

void entity_update(Entity* entity, f32 dt)
{
    EntityState state = entity_context.infos[entity->id].states[entity->state];
    f32 frame_length = state.frame_lengths[entity->frame];
    i32 num_frames = state.num_frames;
    entity->prev_position = entity->position;
    entity->position = vec2_add(entity->position, vec2_scale(entity->direction, entity->speed * dt));
    entity->state_timer += dt;
    entity->frame_timer += entity->frame_speed * dt;
    if (entity->frame_timer > frame_length) {
        entity->frame_timer -= frame_length;
        entity->frame = (entity->frame + 1) % num_frames;
    }
    handle_lava(entity, dt);
    if (entity_get_flag(entity, ENTITY_FLAG_UPDATE_FACING) && vec2_mag(entity->direction) > 0)
        entity->facing = entity->direction;

    entity_context.infos[entity->id].update(&global_api, entity, dt);
    entity_context.infos[entity->id].states[entity->state].update(&global_api, entity, dt);
}

void entity_make_boss(Entity* entity)
{
    list_append(game_context.bosses, entity);
    log_assert(!entity_get_flag(entity, ENTITY_FLAG_BOSS), "Entity is already boss");
    entity_set_flag(entity, ENTITY_FLAG_BOSS, 1);
    entity_boss_update(entity);
    pthread_mutex_lock(&game_context.getter_mutex);
    game_context.values.num_bosses = 1;
    game_context.values.boss_health = entity->health;
    game_context.values.boss_max_health = entity->max_health;
    pthread_mutex_unlock(&game_context.getter_mutex);
}

void entity_boss_update(Entity* entity)
{
    pthread_mutex_lock(&game_context.getter_mutex);
    game_context.values.num_bosses = 1;
    game_context.values.boss_health = entity->health;
    game_context.values.boss_max_health = entity->max_health;
    pthread_mutex_unlock(&game_context.getter_mutex);
}

void entity_unmake_boss(Entity* entity)
{
    i32 idx = list_search(game_context.bosses, entity);
    log_assert(idx != -1, "Entity %p was not found in boss list", entity);
    list_remove(game_context.bosses, idx);
    entity_set_flag(entity, ENTITY_FLAG_BOSS, 0);
    pthread_mutex_lock(&game_context.getter_mutex);
    game_context.values.num_bosses = 0;
    game_context.values.boss_health = 0;
    game_context.values.boss_max_health = 0;
    pthread_mutex_unlock(&game_context.getter_mutex);
}

void entity_set_flag(Entity* entity, EntityFlagEnum flag, bool val)
{
    entity->flags = (entity->flags & ~(1<<flag)) | (val<<flag);
}

bool entity_get_flag(Entity* entity, EntityFlagEnum flag)
{
    return (entity->flags >> flag) & 1;
}

static i32 get_direction_4(f32 rad)
{
    rad = fmod(rad, 2*PI);
    if (rad < 0) rad += 2*PI;
    if (rad > 7 * PI / 4 + 0.01 || rad < PI / 4 - 0.01)
        return UP;
    if (rad < 3 * PI / 4 + 0.01)
        return LEFT;
    if (rad < 5 * PI / 4 - 0.01)
        return DOWN;
    return RIGHT;
}

static i32 get_direction_2(f32 rad)
{
    rad = fmod(rad, 2*PI);
    rad -= PI / 2;
    if (rad < 0) rad += 2*PI;
    return (rad < PI) ? LEFT : RIGHT;
}


i32 entity_get_direction(Entity* entity)
{
    f32 entity_rad = vec2_radians(entity->facing);
    f32 camera_rad = game_context.camera.yaw;
    f32 rad = get_direction_4(entity_rad - camera_rad);
    if (entity_context.infos[entity->id].bidirectional)
        return get_direction_2(rad);
    return get_direction_4(rad);
}

i32 entity_get_texture(Entity* entity)
{
    EntityInfo info = entity_context.infos[entity->id];
    log_assert(entity->state < info.num_states, "Invalid state");
    EntityState state = info.states[entity->state];
    log_assert(entity->frame < state.num_frames, "Invalid frame");
    f32 rad = vec2_radians(entity->facing) - camera_get_yaw();
    i32 dir;
    if (entity_context.infos[entity->id].bidirectional)
        dir = get_direction_2(rad);
    else
        dir = get_direction_4(rad);
    i32 num_frames = state.num_frames;
    return state.frames[num_frames * dir + entity->frame];
}

void entity_destroy(Entity* entity)
{
    entity_context.infos[entity->id].destroy(&global_api, entity);
    if (entity_get_flag(entity, ENTITY_FLAG_BOSS))
        entity_unmake_boss(entity); 
    st_free(entity);
}

void entity_cleanup(void)
{
    if (game_context.entities != NULL) {
        for (i32 i = 0; i < game_context.entities->length; i++)
            entity_destroy(list_get(game_context.entities, i));
        list_destroy(game_context.entities);
    }
    if (game_context.bosses != NULL)
        list_destroy(game_context.bosses);
    for (i32 i = 0; i < entity_context.num_entities; i++) {
        st_free(entity_context.infos[i].name);
        for (i32 j = 0; j < entity_context.infos[i].num_states; j++) {
            st_free(entity_context.infos[i].states[j].name);
            st_free(entity_context.infos[i].states[j].frames);
            st_free(entity_context.infos[i].states[j].frame_lengths);
        }
        st_free(entity_context.infos[i].states);
    }
    st_free(entity_context.infos);
}
