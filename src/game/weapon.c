#include "internal.h"
#include <json.h>
#include <string.h>
#include "../api.h"
#include "../state.h"

typedef void (*WeaponShootFuncPtr)(GlobalApi*, Player*, vec2, vec2);

typedef struct {
    char* name;
    char* tooltip;
    i32 tex_id;
    WeaponShootFuncPtr shoot;
} WeaponInfo;

typedef struct {
    WeaponInfo* infos;
    i32 num_weapons;
} WeaponContext;

static WeaponContext weapon_context;

static void load_tooltip(JsonObject* object, i32 id)
{
    JsonValue* val_string = json_get_value(object, "tooltip");
    char* string = json_get_string(val_string);
    weapon_context.infos[id].tooltip = copy_string(string);
}

static void load_texture(JsonObject* object, i32 id)
{
    JsonValue* val_string = json_get_value(object, "texture");
    char* string = json_get_string(val_string);
    weapon_context.infos[id].tex_id = texture_get_id(string);
}

static void load_attack_func(JsonObject* object, i32 id)
{
    JsonValue* val_string = json_get_value(object, "attack");
    char* string = json_get_string(val_string);
    weapon_context.infos[id].shoot = state_load_function(string);
}

static void load_weapon_info(void)
{
    JsonObject* json = json_read("config/weapons.json");
    JsonIterator* it = json_iterator_create(json);
    JsonMember* member;
    JsonValue* val_object;
    JsonObject* object;
    const char* string;
    weapon_context.num_weapons = json_object_length(json);
    weapon_context.infos = st_malloc(weapon_context.num_weapons * sizeof(WeaponInfo));

    for (i32 i = 0; i < weapon_context.num_weapons; i++) {
        member = json_iterator_get(it);
        string = json_member_key(member);
        weapon_context.infos[i].name = copy_string(string);
        val_object = json_member_value(member);
        object = json_get_object(val_object);
        load_tooltip(object, i);
        load_texture(object, i);
        load_attack_func(object, i);
        json_iterator_increment(it);
    }

    json_iterator_destroy(it);
    json_object_destroy(json);
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
    weapon_context.infos[id].shoot(&global_api, player, direction, target);
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
