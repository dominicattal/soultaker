#include <json.h>
#include <string.h>
#include "../game.h"
#include "../renderer.h"
#include "../api.h"
#include "../state.h"

typedef void (*WeaponShootFuncPtr)(GameApi*, Player*, vec2, vec2);

typedef struct {
    char* name;
    char* tooltip;
    i32 tex_id;
    WeaponShootFuncPtr shoot;
} WeaponInfo;

typedef struct {
    WeaponInfo* infos;
    i32 num_weapons;

    // error handling
    const char* current_weapon;
} WeaponContext;

static WeaponContext weapon_context;

typedef enum {
    ERROR_GENERIC,
    ERROR_CONFIG_FILE,
    ERROR_MISSING,
    ERROR_INVALID_TYPE
} WeaponError;

static void _throw_weapon_error(WeaponError error, i32 line)
{
    const char* name = weapon_context.current_weapon;
    if (name == NULL)
        name = "n/a";
    const char* message;

    switch (error) {
        case ERROR_GENERIC:
            message = "error";
            break;
        case ERROR_CONFIG_FILE:
            message = "could not load weapon config file";
            break;
        case ERROR_MISSING:
            message = "missing field";
            break;
        case ERROR_INVALID_TYPE:
            message = "wrong type";
            break;
    }

    log_write(FATAL, "%s:%d\nweapon: %s\n%s", 1024, __FILE__, line, name, message);
}

#define throw_weapon_error(error) \
    _throw_weapon_error(error, __LINE__);

static void load_tooltip(JsonObject* object, i32 id)
{
    JsonValue* val_string = json_object_get_value(object, "tooltip");
    if (json_value_get_type(val_string) != JTYPE_STRING)
        throw_weapon_error(ERROR_INVALID_TYPE);

    char* string = json_value_get_string(val_string);
    if (string == NULL)
        throw_weapon_error(ERROR_MISSING);

    weapon_context.infos[id].tooltip = string_copy(string);
}

static void load_texture(JsonObject* object, i32 id)
{
    JsonValue* val_string = json_object_get_value(object, "texture");
    if (json_value_get_type(val_string) != JTYPE_STRING)
        throw_weapon_error(ERROR_INVALID_TYPE);

    char* string = json_value_get_string(val_string);
    if (string == NULL)
        throw_weapon_error(ERROR_MISSING);

    weapon_context.infos[id].tex_id = texture_get_id(string);
}

static void load_attack_func(JsonObject* object, i32 id)
{
    JsonValue* val_string = json_object_get_value(object, "attack");
    if (json_value_get_type(val_string) != JTYPE_STRING)
        throw_weapon_error(ERROR_INVALID_TYPE);

    char* string = json_value_get_string(val_string);
    if (string == NULL)
        throw_weapon_error(ERROR_MISSING);

    weapon_context.infos[id].shoot = state_load_function(string);
}

static void load_weapon_info(void)
{
    JsonObject* json = state_context.config->weapons;
    JsonIterator* it = json_iterator_create(json);
    if (it == NULL)
        throw_weapon_error(ERROR_GENERIC);

    JsonMember* member;
    JsonValue* val_object;
    JsonObject* object;
    const char* string;
    weapon_context.num_weapons = json_object_length(json);
    weapon_context.infos = st_malloc(weapon_context.num_weapons * sizeof(WeaponInfo));

    for (i32 i = 0; i < weapon_context.num_weapons; i++) {
        member = json_iterator_get(it);
        string = json_member_get_key(member);
        weapon_context.infos[i].name = string_copy(string);
        val_object = json_member_get_value(member);
        object = json_value_get_object(val_object);
        load_tooltip(object, i);
        load_texture(object, i);
        load_attack_func(object, i);
        json_iterator_increment(it);
    }

    json_iterator_destroy(it);
}

void weapon_init(void)
{
    load_weapon_info();
}

void weapon_cleanup(void)
{
    for (i32 i = 0; i < weapon_context.num_weapons; i++) {
        st_free(weapon_context.infos[i].name);
        st_free(weapon_context.infos[i].tooltip);
    }
    st_free(weapon_context.infos);
}

i32 weapon_get_id(const char* name)
{
    i32 l, r, m, a;
    l = 0;
    r = weapon_context.num_weapons-1;
    while (l <= r) {
        m = l + (r - l) / 2;
        a = strcmp(name, weapon_context.infos[m].name);
        if (a > 0)
            l = m + 1;
        else if (a < 0)
            r = m - 1;
        else
            return m;
    }
    log_write(FATAL, "Could not get id for %s", name);
    return -1;
}

void weapon_shoot(Player* player, vec2 direction, vec2 target)
{
    i32 id = player->weapon.id;
    weapon_context.infos[id].shoot(&game_api, player, direction, target);
}

char* weapon_get_name(i32 id)
{
    return weapon_context.infos[id].name;
}

char* weapon_get_tooltip(i32 id)
{
    return weapon_context.infos[id].tooltip;
}

i32 weapon_get_tex_id(i32 id)
{
    return weapon_context.infos[id].tex_id;
}
