#include "internal.h"
#include <json.h>

typedef struct {
    char* name;
    char* tooltip;
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
        json_iterator_increment(it);
    }

    json_iterator_destroy(it);
    json_object_destroy(json);
}

void weapon_init(void)
{
    log_write(INFO, "Initializing weapons...");
    load_weapon_info();
    log_write(INFO, "Initalized weapons");
}

void weapon_cleanup(void)
{
    for (i32 i = 0; i < weapon_context.num_weapons; i++) {
        st_free(weapon_context.infos[i].name);
        st_free(weapon_context.infos[i].tooltip);
    }
    st_free(weapon_context.infos);
}
