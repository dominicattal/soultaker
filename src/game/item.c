#include <json.h>
#include <string.h>
#include "../game.h"
#include "../renderer.h"
#include "../api.h"
#include "../state.h"

typedef void (*WeaponShootFuncPtr)(GameApi*, Player*, vec2, vec2);

typedef struct {
    ItemTypeEnum type;
    char* name;
    char* tooltip;
    char* display_name;
    i32 tex_id;
    WeaponShootFuncPtr shoot;
} ItemInfo;

typedef struct {
    ItemInfo* infos;
    i32 num_items;

    // error handling
    const char* current_item;
} ItemContext;

static ItemContext item_context;

typedef enum {
    ERROR_GENERIC,
    ERROR_CONFIG_FILE,
    ERROR_MISSING,
    ERROR_INVALID_TYPE,
    ERROR_INVALID_ITEM_TYPE
} ItemError;

static void _throw_item_error(ItemError error, i32 line)
{
    const char* name = item_context.current_item;
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

    log_write(FATAL, "%s:%d\nitem: %s\n%s", 1024, __FILE__, line, name, message);
}

#define throw_item_error(error) \
    _throw_item_error(error, __LINE__);

static void load_type(JsonObject* object, i32 id)
{
    JsonValue* val_string = json_object_get_value(object, "type");
    if (val_string == NULL)
        throw_item_error(ERROR_GENERIC);
    if (json_value_get_type(val_string) != JTYPE_STRING)
        throw_item_error(ERROR_INVALID_TYPE);

    char* string = json_value_get_string(val_string);
    if (string == NULL)
        throw_item_error(ERROR_MISSING);

    if (strcmp(string, "weapon") == 0)
        item_context.infos[id].type = ITEM_WEAPON;
    else if (strcmp(string, "armor") == 0)
        item_context.infos[id].type = ITEM_ARMOR;
    else if (strcmp(string, "equipment") == 0)
        item_context.infos[id].type = ITEM_EQUIPMENT;
    else if (strcmp(string, "material") == 0)
        item_context.infos[id].type = ITEM_MATERIAL;
    else
        throw_item_error(ERROR_INVALID_ITEM_TYPE);
}

static void load_display_name(JsonObject* object, i32 id)
{
    JsonValue* val_string = json_object_get_value(object, "display_name");
    if (json_value_get_type(val_string) != JTYPE_STRING)
        throw_item_error(ERROR_INVALID_TYPE);

    char* string = json_value_get_string(val_string);
    if (string == NULL)
        throw_item_error(ERROR_MISSING);

    item_context.infos[id].display_name = string_copy(string);
}

static void load_tooltip(JsonObject* object, i32 id)
{
    JsonValue* val_string = json_object_get_value(object, "tooltip");
    if (val_string == NULL)
        throw_item_error(ERROR_GENERIC);
    if (json_value_get_type(val_string) != JTYPE_STRING)
        throw_item_error(ERROR_INVALID_TYPE);

    char* string = json_value_get_string(val_string);
    if (string == NULL)
        throw_item_error(ERROR_MISSING);

    item_context.infos[id].tooltip = string_copy(string);
}

static void load_texture(JsonObject* object, i32 id)
{
    JsonValue* val_string = json_object_get_value(object, "texture");
    if (val_string == NULL)
        throw_item_error(ERROR_GENERIC);
    if (json_value_get_type(val_string) != JTYPE_STRING)
        throw_item_error(ERROR_INVALID_TYPE);

    char* string = json_value_get_string(val_string);
    if (string == NULL)
        throw_item_error(ERROR_MISSING);

    item_context.infos[id].tex_id = texture_get_id(string);
}

static void load_attack_func(JsonObject* object, i32 id)
{
    JsonValue* val_string = json_object_get_value(object, "attack");
    if (val_string == NULL)
        return;
    if (json_value_get_type(val_string) != JTYPE_STRING)
        throw_item_error(ERROR_INVALID_TYPE);

    char* string = json_value_get_string(val_string);
    if (string == NULL)
        throw_item_error(ERROR_MISSING);

    item_context.infos[id].shoot = state_load_function(string);
}

static void load_item_info(void)
{
    JsonObject* json = state_context.config->items;
    JsonIterator* it = json_iterator_create(json);
    if (it == NULL)
        throw_item_error(ERROR_GENERIC);

    JsonMember* member;
    JsonValue* val_object;
    JsonObject* object;
    const char* string;
    item_context.num_items = json_object_length(json);
    item_context.infos = st_malloc(item_context.num_items * sizeof(ItemInfo));

    for (i32 i = 0; i < item_context.num_items; i++) {
        member = json_iterator_get(it);
        string = json_member_get_key(member);
        item_context.infos[i].name = string_copy(string);
        val_object = json_member_get_value(member);
        object = json_value_get_object(val_object);
        load_tooltip(object, i);
        load_display_name(object, i);
        load_type(object, i);
        load_texture(object, i);
        load_attack_func(object, i);
        json_iterator_increment(it);
    }

    json_iterator_destroy(it);
}

void item_init(void)
{
    load_item_info();
}

void item_cleanup(void)
{
    for (i32 i = 0; i < item_context.num_items; i++) {
        st_free(item_context.infos[i].name);
        st_free(item_context.infos[i].tooltip);
        st_free(item_context.infos[i].display_name);
    }
    st_free(item_context.infos);
}

i32 item_get_id(const char* name)
{
    i32 l, r, m, a;
    l = 0;
    r = item_context.num_items-1;
    while (l <= r) {
        m = l + (r - l) / 2;
        a = strcmp(name, item_context.infos[m].name);
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

i32 item_get_tex_id(i32 item_id)
{
    return item_context.infos[item_id].tex_id;
}

void inventory_swap_items(Item** item1, Item** item2)
{
    Item* tmp = *item1;
    *item1 = *item2;
    *item2 = tmp;
}

void weapon_shoot(Player* player, vec2 direction, vec2 target)
{
    Item* item = *player->inventory.item_weapon;
    if (item == NULL) {
        log_write(WARNING, "Weapon does not exist");
        return;
    }
    if (item->type != ITEM_WEAPON) {
        log_write(WARNING, "Equipped item not weapon");
        return;
    }
    i32 id = item->id;
    item_context.infos[id].shoot(&game_api, player, direction, target);
}

char* weapon_get_name(i32 id)
{
    return item_context.infos[id].name;
}

char* weapon_get_tooltip(i32 id)
{
    return item_context.infos[id].tooltip;
}

Item* item_create(i32 id)
{
    Item* item = st_malloc(sizeof(Item));
    item->id = id;
    item->type = item_context.infos[id].type;
    item->equipped = false;
    for (i32 i = 0; i < NUM_STATS; i++) {
        item->additive_stats[i] = 0;
        item->multiplicative_stats[i] = 0;
    }
    return item;
}

char* item_get_display_name(Item* item)
{
    return item_context.infos[item->id].display_name;
}

char* item_get_tooltip(Item* item)
{
    return item_context.infos[item->id].tooltip;
}

void item_destroy(Item* item)
{
    st_free(item);
}
