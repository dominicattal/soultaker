#include "internal.h"
#include "../state.h"
#include "../api.h"
#include <json.h>
#include <windows.h>
#include <stb_image.h>

typedef void (*MapLoadFuncPtr)(GlobalApi*);

typedef struct {
    MapLoadFuncPtr load;
    char* name;
} MapInfo;

typedef struct {
    MapInfo* maps;
    i32 num_maps;
} MapContext;

extern GameContext game_context;
MapContext map_context;

void map_init(void)
{
    JsonObject* json = json_read("config/maps.json");
    log_assert(json, "Could not read map config file");
    JsonIterator* it = json_iterator_create(json);
    log_assert(json, "Could not create iterator for map config file");
    JsonMember* member;
    JsonValue* val_string;
    const char* string;
    map_context.num_maps = json_object_length(json);
    map_context.maps = st_malloc(map_context.num_maps * sizeof(MapInfo));
    for (i32 i = 0; i < map_context.num_maps; i++) {
        member = json_iterator_get(it);
        log_assert(member, "Could not get member from map config file");

        string = json_member_key(member);
        log_assert(string, "Could not get key from map config file");
        map_context.maps[i].name = string_copy(string);

        val_string = json_member_value(member);
        log_assert(val_string, "Could not get value from member");
        log_assert(json_get_type(val_string) == JTYPE_STRING, "Value must be a string");
        string = json_get_string(val_string);
        log_assert(string, "Could not get string from member");
        map_context.maps[i].load = state_load_function(string);
        log_assert(map_context.maps[i].load, "Could not find function %s in library", string);

        json_iterator_increment(it);
    }

    json_iterator_destroy(it);
    json_object_destroy(json);
}

i32 map_get_id(const char* name)
{
    int l, r, m, a;
    l = 0;
    r = map_context.num_maps-1;
    while (l <= r) {
        m = l + (r - l) / 2;
        a = strcmp(name, map_context.maps[m].name);
        if (a > 0)
            l = m + 1;
        else if (a < 0)
            r = m - 1;
        else
            return m;
    }
    log_write(WARNING, "Could not map map %s", name);
    return -1;
}

void map_load(i32 id)
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
    game_render_update_obstacles();
    game_render_update_parstacles();
    game_render_update_tiles();
    game_render_update_walls();
    map_context.maps[id].load(&global_api);
}

void map_cleanup(void)
{
    for (i32 i = 0; i < map_context.num_maps; i++) 
        st_free(map_context.maps[i].name);
    st_free(map_context.maps);
}

Map* map_create(const char* path)
{
    i32 x, y, n, i, j, idx;
    u64 map_area;
    unsigned char* data = stbi_load(path, &x, &y, &n, 3);
    if (data == NULL)
        log_write(FATAL, "Could not load map data for %s", path);
    Map* map = st_malloc(sizeof(Map));
    map->width = x;
    map->length = y;
    map_area = map->width * map->length;
    map->data = st_malloc(map_area * sizeof(u32));
    for (i = 0; i < map->length; i++) {
        for (j = 0; j < map->width; j++) {
            idx = (map->length-i-1) * map->width + j;
            map->data[i*map->width+j] = data[3*idx+2] + (data[3*idx+1]<<8) + (data[3*idx]<<16);
        }
    }
    stbi_image_free(data);
    return map;
}

void map_free(Map* map)
{
    st_free(map->data);
    st_free(map);
}
