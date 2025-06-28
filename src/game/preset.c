#include "internal.h"
#include "../state.h"
#include "../api.h"
#include <json.h>
#include <windows.h>

typedef void (*PresetLoadFuncPtr)(GlobalApi*);

typedef struct {
    PresetLoadFuncPtr load;
    char* name;
} PresetInfo;

typedef struct {
    PresetInfo* presets;
    i32 num_presets;
} PresetContext;

extern GameContext game_context;
PresetContext preset_context;

void game_preset_init(void)
{
    JsonObject* json = json_read("config/presets.json");
    log_assert(json, "Could not read preset config file");
    JsonIterator* it = json_iterator_create(json);
    log_assert(json, "Could not create iterator for preset config file");
    JsonMember* member;
    JsonValue* val_string;
    const char* string;
    preset_context.num_presets = json_object_length(json);
    preset_context.presets = st_malloc(preset_context.num_presets * sizeof(PresetInfo));
    for (i32 i = 0; i < preset_context.num_presets; i++) {
        member = json_iterator_get(it);
        log_assert(member, "Could not get member from preset config file");

        string = json_member_key(member);
        log_assert(string, "Could not get key from preset config file");
        preset_context.presets[i].name = copy_string(string);

        val_string = json_member_value(member);
        log_assert(val_string, "Could not get value from member");
        log_assert(json_get_type(val_string) == JTYPE_STRING, "Value must be a string");
        string = json_get_string(val_string);
        log_assert(string, "Could not get string from member");
        preset_context.presets[i].load = state_load_function(string);
        log_assert(preset_context.presets[i].load, "Could not find function %s in library", string);

        json_iterator_increment(it);
    }

    json_iterator_destroy(it);
    json_object_destroy(json);
}

i32 game_preset_map_id(const char* name)
{
    int l, r, m, a;
    l = 0;
    r = preset_context.num_presets-1;
    while (l <= r) {
        m = l + (r - l) / 2;
        a = strcmp(name, preset_context.presets[m].name);
        if (a > 0)
            l = m + 1;
        else if (a < 0)
            r = m - 1;
        else
            return m;
    }
    log_write(WARNING, "Could not map preset %s", name);
    return -1;
}

void game_preset_load(i32 id)
{
    entity_clear();
    tile_clear();
    wall_clear();
    projectile_clear();
    parstacle_clear();
    obstacle_clear();
    particle_clear();
    parjicle_clear();
    player_reset();
    game_context.data.update_tile_buffer = true;
    game_context.data.update_wall_buffer = true;
    game_context.data.update_parstacle_buffer = true;
    game_context.data.update_obstacle_buffer = true;
    game_context.data_swap.update_tile_buffer = true;
    game_context.data_swap.update_wall_buffer = true;
    game_context.data_swap.update_parstacle_buffer = true;
    game_context.data_swap.update_obstacle_buffer = true;
    preset_context.presets[id].load(&global_api);
}

void game_preset_cleanup(void)
{
    for (i32 i = 0; i < preset_context.num_presets; i++) 
        st_free(preset_context.presets[i].name);
    st_free(preset_context.presets);
}
