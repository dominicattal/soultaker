#include "../game.h"
#include "../state.h"
#include <string.h>

extern GameContext game_context;
SynergyContext synergy_context;

typedef enum {
    ERROR_GENERIC,
    ERROR_CONFIG_FILE,
    ERROR_MISSING,
    ERROR_INVALID_TYPE,
    ERROR_INVALID_ITEM_TYPE
} SynergyError;

static void _throw_synergy_error(SynergyError error, i32 line)
{
    const char* name = synergy_context.current_synergy;
    if (name == NULL)
        name = "n/a";
    const char* message;

    switch (error) {
        case ERROR_GENERIC:
            message = "error";
            break;
        case ERROR_CONFIG_FILE:
            message = "could not load item config file";
            break;
        case ERROR_MISSING:
            message = "missing field";
            break;
        case ERROR_INVALID_TYPE:
            message = "wrong type";
            break;
        case ERROR_INVALID_ITEM_TYPE:
            message = "Invalid item type";
            break;
    }

    log_write(FATAL, "%s:%d\nitem: %s\n%s", __FILE__, line, name, message);
}

#define throw_synergy_error(error) \
    _throw_synergy_error(error, __LINE__);

static void load_display_name(JsonObject* object, i32 id)
{
    JsonValue* val_string = json_object_get_value(object, "display_name");
    if (val_string == NULL)
        throw_synergy_error(ERROR_GENERIC);
    if (json_value_get_type(val_string) != JTYPE_STRING)
        throw_synergy_error(ERROR_INVALID_TYPE);

    char* string = json_value_get_string(val_string);
    if (string == NULL)
        throw_synergy_error(ERROR_MISSING);

    synergy_context.infos[id].display_name = string_copy(string);
}

static void load_tooltip(JsonObject* object, i32 id)
{
    JsonValue* val_string = json_object_get_value(object, "tooltip");
    if (val_string == NULL)
        throw_synergy_error(ERROR_GENERIC);
    if (json_value_get_type(val_string) != JTYPE_STRING)
        throw_synergy_error(ERROR_INVALID_TYPE);

    char* string = json_value_get_string(val_string);
    if (string == NULL)
        throw_synergy_error(ERROR_MISSING);

    synergy_context.infos[id].tooltip = string_copy(string);
}

static void load_primary_func(JsonObject* object, i32 id)
{
    JsonValue* val_string = json_object_get_value(object, "primary");
    if (val_string == NULL) {
        synergy_context.infos[id].primary = NULL;
        return;
    }
    if (json_value_get_type(val_string) != JTYPE_STRING)
        throw_synergy_error(ERROR_INVALID_TYPE);

    char* string = json_value_get_string(val_string);
    if (string == NULL)
        throw_synergy_error(ERROR_MISSING);

    synergy_context.infos[id].primary = state_load_function(string);
}

static void load_secondary_func(JsonObject* object, i32 id)
{
    JsonValue* val_string = json_object_get_value(object, "secondary");
    if (val_string == NULL) {
        synergy_context.infos[id].secondary = NULL;
        return;
    }
    if (json_value_get_type(val_string) != JTYPE_STRING)
        throw_synergy_error(ERROR_INVALID_TYPE);

    char* string = json_value_get_string(val_string);
    if (string == NULL)
        throw_synergy_error(ERROR_MISSING);

    synergy_context.infos[id].secondary = state_load_function(string);
}

static void load_cast_func(JsonObject* object, i32 id)
{
    JsonValue* val_string = json_object_get_value(object, "cast");
    if (val_string == NULL) {
        synergy_context.infos[id].cast = NULL;
        return;
    }
    if (json_value_get_type(val_string) != JTYPE_STRING)
        throw_synergy_error(ERROR_INVALID_TYPE);

    char* string = json_value_get_string(val_string);
    if (string == NULL)
        throw_synergy_error(ERROR_MISSING);

    synergy_context.infos[id].cast = state_load_function(string);
}

static void load_use_func(JsonObject* object, i32 id)
{
    JsonValue* val_string = json_object_get_value(object, "use");
    if (val_string == NULL) {
        synergy_context.infos[id].use = NULL;
        return;
    }
    if (json_value_get_type(val_string) != JTYPE_STRING)
        throw_synergy_error(ERROR_INVALID_TYPE);

    char* string = json_value_get_string(val_string);
    if (string == NULL)
        throw_synergy_error(ERROR_MISSING);

    synergy_context.infos[id].use = state_load_function(string);
}

static void load_items(JsonObject* object, i32 id)
{
    JsonValue* val_array = json_object_get_value(object, "items");
    if (val_array == NULL)
        throw_synergy_error(ERROR_MISSING);
    if (json_value_get_type(val_array) != JTYPE_ARRAY)
        throw_synergy_error(ERROR_INVALID_TYPE);
    
    JsonValue* val_string;
    char* name;
    i32 item_id;
    JsonArray* array = json_value_get_array(val_array);
    SynergyInfo* info = &synergy_context.infos[id];
    info->num_items = json_array_length(array);
    info->item_ids = st_malloc(info->num_items * sizeof(i32));
    for (i32 i = 0; i < info->num_items; i++) {
        val_string = json_array_get(array, i);
        if (json_value_get_type(val_string) != JTYPE_STRING)
            throw_synergy_error(ERROR_INVALID_TYPE);
        name = json_value_get_string(val_string);
        item_id = item_get_id(name);
        if (item_id == -1)
            log_write(FATAL, "item doesn't exist");
        info->item_ids[i] = item_id;
    }
}

static void load_synergy_info(void)
{
    JsonObject* json = state_context.config->synergies;
    JsonIterator* it = json_iterator_create(json);
    if (it == NULL)
        throw_synergy_error(ERROR_GENERIC);

    JsonMember* member;
    JsonValue* val_object;
    JsonObject* object;
    const char* string;
    synergy_context.num_synergies = json_object_length(json);
    synergy_context.infos = st_malloc(synergy_context.num_synergies * sizeof(SynergyInfo));

    for (i32 i = 0; i < synergy_context.num_synergies; i++) {
        member = json_iterator_get(it);
        string = json_member_get_key(member);
        synergy_context.infos[i].name = string_copy(string);
        val_object = json_member_get_value(member);
        object = json_value_get_object(val_object);
        load_tooltip(object, i);
        //load_display_name(object, i);
        load_primary_func(object, i);
        load_secondary_func(object, i);
        load_cast_func(object, i);
        load_use_func(object, i);
        load_items(object, i);
        json_iterator_increment(it);
    }

    json_iterator_destroy(it);
}

void synergy_init(void)
{
    load_synergy_info();
}

Synergy* synergy_create(i32 id)
{
    Synergy* synergy = st_malloc(sizeof(Synergy));
    synergy->primary_cooldown = 0.33;
    synergy->primary_timer = 0;
    synergy->secondary_cooldown = 0.33;
    synergy->secondary_timer = 0;
    synergy->cast_cooldown = 0;
    synergy->cast_timer = 0;
    synergy->use_cooldown = 0;
    synergy->use_timer = 0;
    synergy->id = id;
    return synergy;
}

void synergy_update(Synergy* synergy, f32 dt)
{
    synergy->primary_timer -= dt;
    if (synergy->primary_timer < 0)
        synergy->primary_timer = 0;
    synergy->secondary_timer -= dt;
    if (synergy->secondary_timer < 0)
        synergy->secondary_timer = 0;
}

void synergy_cleanup(void)
{
    for (i32 i = 0; i < synergy_context.num_synergies; i++) {
        st_free(synergy_context.infos[i].name);
        st_free(synergy_context.infos[i].tooltip);
        st_free(synergy_context.infos[i].item_ids);
        //st_free(synergy_context.infos[i].display_name);
    }
    st_free(synergy_context.infos);
}

i32 synergy_get_id(const char* name)
{
    i32 l, r, m, a;
    l = 0;
    r = synergy_context.num_synergies-1;
    while (l <= r) {
        m = l + (r - l) / 2;
        a = strcmp(name, synergy_context.infos[m].name);
        if (a > 0)
            l = m + 1;
        else if (a < 0)
            r = m - 1;
        else
            return m;
    }
    log_write(CRITICAL, "Could not get id for %s", name);
    return -1;
}

char* synergy_get_name(i32 id)
{
    return synergy_context.infos[id].name;
}
