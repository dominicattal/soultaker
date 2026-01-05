#include "internal.h"
#include "../state.h"
#include "../api.h"
#include "../event.h"
#include <json.h>
#include <string.h>
#include <math.h>

#define DEFAULT_FRAME_LENGTH 0.5

extern GameContext game_context;

typedef void (*CreateFuncPtr)(GameApi*, Entity*);
typedef void (*DestroyFuncPtr)(GameApi*, Entity*);
typedef void (*UpdateFuncPtr)(GameApi*, Entity*, f32);

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

    // error handling when loading from config file
    const char* current_entity_name;
    const char* current_state_name;
    const char* current_function_name;
} EntityContext;

static EntityContext entity_context;

typedef enum {
    ERROR_GENERIC,
    ERROR_INVALID_TYPE,
    ERROR_MISSING,
    ERROR_LOAD_FAILURE,
    ERROR_INVALID_COUNT,
    ERROR_CONFIG_FILE
} EntityError;

static void _throw_entity_error(EntityError error, i32 line)
{
    const char* name = entity_context.current_entity_name;
    if (name == NULL) 
        name = "n/a";
    const char* state = entity_context.current_state_name;
    if (state == NULL)
        state = "n/a";
    const char* func = entity_context.current_function_name;
    if (func == NULL)
        func = "n/a";
    const char* message;

    switch (error) {
        case ERROR_GENERIC:
            message = "generic error";
            break;
        case ERROR_INVALID_TYPE:
            message = "invalid type";
            break;
        case ERROR_MISSING:
            message = "missing something";
            break;
        case ERROR_LOAD_FAILURE:
            message = "could not load function";
            break;
        case ERROR_INVALID_COUNT:
            message = "frame count does not match";
            break;
        case ERROR_CONFIG_FILE:
            message = "could not load config file";
            break;
    }

    char* error_string = string_create("%s:%d\nentity: %s\nstate: %s\nfunction: %s\n%s", 1024, __FILE__, line, name, state, func, message);
    log_write(FATAL, error_string);
}

#define throw_entity_error(error) \
    _throw_entity_error(error, __LINE__)

static void* load_function(JsonObject* object, const char* key)
{
    JsonValue* val_string = json_object_get_value(object, key);

    if (val_string == NULL)
        return NULL;
    
    if (json_value_get_type(val_string) != JTYPE_STRING)
        throw_entity_error(ERROR_INVALID_TYPE);

    const char* function_name = json_value_get_string(val_string);
    if (function_name == NULL)
        throw_entity_error(ERROR_MISSING);

    entity_context.current_function_name = function_name;
    void* fptr = state_load_function(function_name);
    if (fptr == NULL)
        throw_entity_error(ERROR_LOAD_FAILURE);

    entity_context.current_function_name = NULL;
    return fptr;
}

static void load_state_frame_lengths(EntityState* state, JsonObject* object)
{
    JsonValue* value;
    JsonArray* array;
    i32 int_val, i;
    f32 float_val;
    value = json_object_get_value(object, "timers");
    if (value == NULL) {
        for (i = 0; i < state->num_frames; i++)
            state->frame_lengths[i] = DEFAULT_FRAME_LENGTH;
        return;
    }

    if (json_value_get_type(value) != JTYPE_ARRAY)
        throw_entity_error(ERROR_INVALID_TYPE);

    array = json_value_get_array(value);
    int_val  = json_array_length(array);
    if (int_val != state->num_frames)
        throw_entity_error(ERROR_INVALID_COUNT);

    for (i = 0; i < state->num_frames; i++) {
        value = json_array_get(array, i);
        if (value == NULL)
            throw_entity_error(ERROR_MISSING);
        if (json_value_get_type(value) != JTYPE_FLOAT)
            throw_entity_error(ERROR_INVALID_TYPE);
        float_val = json_value_get_float(value);
        state->frame_lengths[i] = float_val;
    }
}

static void load_state_frames(EntityState* state, JsonObject* object, const char* dir_str, i32 dir_int)
{
    JsonValue* value;
    JsonArray* array;
    const char* string;
    i32 int_val;
    i32 num_frames = state->num_frames;
    value = json_object_get_value(object, dir_str);
    if (value == NULL)
        throw_entity_error(ERROR_MISSING);
    if (json_value_get_type(value) != JTYPE_ARRAY)
        throw_entity_error(ERROR_INVALID_TYPE);

    array = json_value_get_array(value);
    int_val = json_array_length(array);
    if (int_val != num_frames)
        throw_entity_error(ERROR_INVALID_COUNT);

    for (i32 j = 0; j < num_frames; j++) {
        value = json_array_get(array, j);
        if (value == NULL)
            throw_entity_error(ERROR_MISSING);
        if (json_value_get_type(value) != JTYPE_STRING)
            throw_entity_error(ERROR_INVALID_TYPE);

        string = json_value_get_string(value);
        if (string == NULL)
            throw_entity_error(ERROR_MISSING);
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
    value = json_object_get_value(object, "bidirectional");
    if (value != NULL)  {
        type = json_value_get_type(value);
        if (type == JTYPE_TRUE)
            bidirectional = 1;
    }

    value = json_object_get_value(object, "states");
    if (value == NULL)
        throw_entity_error(ERROR_MISSING);
    if (json_value_get_type(value) != JTYPE_OBJECT)
        throw_entity_error(ERROR_INVALID_TYPE);

    obj_states = json_value_get_object(value);
    if (obj_states == NULL)
        throw_entity_error(ERROR_MISSING);

    it = json_iterator_create(obj_states);
    i32 num_states = json_object_length(obj_states);
    EntityState* state_ptr = st_malloc(num_states * sizeof(EntityState));

    for (i32 i = 0; i < num_states; i++) {

        member = json_iterator_get(it);
        name = json_member_get_key(member);

        entity_context.current_state_name = name;

        value = json_member_get_value(member);
        if (value == NULL)
            throw_entity_error(ERROR_MISSING);
        if (json_value_get_type(value) != JTYPE_OBJECT)
            throw_entity_error(ERROR_INVALID_TYPE);

        object = json_value_get_object(value);
        if (object == NULL)
            throw_entity_error(ERROR_MISSING);

        value = json_object_get_value(object, "frames");
        if (value == NULL)
            throw_entity_error(ERROR_MISSING);
        if (json_value_get_type(value) != JTYPE_INT)
            throw_entity_error(ERROR_INVALID_TYPE);

        num_frames = json_value_get_int(value);
        if (num_frames <= 0)
            throw_entity_error(ERROR_INVALID_COUNT);

        state_ptr[i].name = string_copy(name);
        state_ptr[i].num_frames = num_frames;
        state_ptr[i].frames = st_malloc(4 * num_frames * sizeof(i32));
        state_ptr[i].frame_lengths = st_malloc(num_frames * sizeof(f32));
        state_ptr[i].update = load_function(object, "update");

        load_state_frame_lengths(&state_ptr[i], object);
        load_state_frames(&state_ptr[i], object, "left", LEFT);
        load_state_frames(&state_ptr[i], object, "right", RIGHT);
        if (!bidirectional) {
            load_state_frames(&state_ptr[i], object, "up", UP);
            load_state_frames(&state_ptr[i], object, "down", DOWN);
        }

        json_iterator_increment(it);
    }

    entity_context.current_state_name = NULL;

    json_iterator_destroy(it);

    entity_context.infos[entity_id].num_states = num_states;
    entity_context.infos[entity_id].states = state_ptr;
    entity_context.infos[entity_id].bidirectional = bidirectional;
}

static void load_entity_info(void)
{
    JsonObject* json = json_read("config/entities.json");
    if (json == NULL)
        throw_entity_error(ERROR_CONFIG_FILE);
    JsonIterator* it = json_iterator_create(json);
    if (it == NULL)
        throw_entity_error(ERROR_GENERIC);

    JsonMember* member;
    JsonValue* val_object;
    JsonObject* object;
    const char* string;
    entity_context.num_entities = json_object_length(json);
    entity_context.infos = st_malloc(entity_context.num_entities * sizeof(EntityInfo));
    for (i32 i = 0; i < entity_context.num_entities; i++) {
        member = json_iterator_get(it);
        if (member == NULL)
            throw_entity_error(ERROR_GENERIC);

        string = json_member_get_key(member);
        if (string == NULL)
            throw_entity_error(ERROR_GENERIC);

        entity_context.infos[i].name = string_copy(string);
        entity_context.current_entity_name = string;

        val_object = json_member_get_value(member);
        if (val_object == NULL)
            throw_entity_error(ERROR_MISSING);
        if (json_value_get_type(val_object) != JTYPE_OBJECT)
            throw_entity_error(ERROR_INVALID_TYPE);

        object = json_value_get_object(val_object);
        if (object == NULL)
            throw_entity_error(ERROR_MISSING);

        entity_context.infos[i].create = load_function(object, "create");
        entity_context.infos[i].destroy = load_function(object, "destroy");
        entity_context.infos[i].update = load_function(object, "update");
        load_state_info(i, object);

        json_iterator_increment(it);
    }

    entity_context.current_entity_name = NULL;
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
    entity->state = entity_get_state_id(entity, name);
    entity->frame = 0;
}

void entity_init(void)
{
    load_entity_info();
}

void entity_clear(void)
{
    //if (game_context.entities == NULL)
    //    return;
    //while (game_context.entities->length > 0)
    //    entity_destroy(list_remove(game_context.entities, 0));
    //game_context.player.entity = NULL;
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

    CreateFuncPtr create = entity_context.infos[id].create;
    if (create != NULL)
        create(&game_api, entity);

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

    UpdateFuncPtr update;
    update = entity_context.infos[entity->id].update;
    if (update != NULL)
        update(&game_api, entity, dt);
    update = entity_context.infos[entity->id].states[entity->state].update;
    if (update != NULL)
        update(&game_api, entity, dt);
}

void entity_damage(Entity* entity, f32 damage)
{
    entity->health -= damage;
    if (!entity_get_flag(entity, ENTITY_FLAG_BOSS))
        return;
    event_create_gui_update_boss_healthbar(entity, entity->health, entity->max_health);
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
    if (rad > 7 * PI / 4 + EPSILON || rad < PI / 4 - EPSILON)
        return UP;
    if (rad < 3 * PI / 4 + EPSILON)
        return LEFT;
    if (rad < 5 * PI / 4 - EPSILON)
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

void entity_set_direction(Entity* entity, vec2 direction)
{
}

void entity_set_facing(Entity* entity, vec2 facing)
{
}

void entity_destroy(Entity* entity)
{
    DestroyFuncPtr destroy = entity_context.infos[entity->id].destroy;
    if (destroy != NULL)
        destroy(&game_api, entity);
    st_free(entity);
}

void entity_cleanup(void)
{
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
