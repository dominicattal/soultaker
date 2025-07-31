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
    
    // error handling
    const char* current_map;
} MapContext;

extern GameContext game_context;
MapContext map_context;

typedef enum {
    ERROR_MISSING,
    ERROR_INVALID_TYPE,
    ERROR_CONFIG_FILE,
    ERROR_GENERIC
} MapError;

static void _throw_map_error(MapError error, i32 line)
{
    const char* name = map_context.current_map;
    if (name == NULL)
        name = "n/a";
    const char* message;

    switch (error) {
        case ERROR_MISSING:
            message = "missing something";
            break;
        case ERROR_INVALID_TYPE:
            message = "wrong type";
            break;
        case ERROR_CONFIG_FILE:
            message = "could not load config file";
            break;
        case ERROR_GENERIC:
            message = "lets fucking go";
            break;
    }

    log_write(FATAL, "%s:%d\nmap: %s\n%s", __FILE__, line, name, message);
}

#define throw_map_error(error) \
    _throw_map_error(error, __LINE__)

void map_init(void)
{
    JsonObject* json = json_read("config/maps.json");
    if (json == NULL)
        throw_map_error(ERROR_CONFIG_FILE);
    JsonIterator* it = json_iterator_create(json);
    if (it == NULL)
        throw_map_error(ERROR_GENERIC);

    JsonMember* member;
    JsonValue* val_string;
    const char* string;
    map_context.num_maps = json_object_length(json);
    map_context.maps = st_malloc(map_context.num_maps * sizeof(MapInfo));
    for (i32 i = 0; i < map_context.num_maps; i++) {
        member = json_iterator_get(it);

        string = json_member_key(member);
        if (string == NULL)
            throw_map_error(ERROR_MISSING);

        map_context.maps[i].name = string_copy(string);
        map_context.current_map = string;

        val_string = json_member_value(member);
        if (val_string == NULL)
            throw_map_error(ERROR_MISSING);
        if (json_get_type(val_string) != JTYPE_STRING)
            throw_map_error(ERROR_INVALID_TYPE);

        string = json_get_string(val_string);
        if (string == NULL)
            throw_map_error(ERROR_MISSING);

        map_context.maps[i].load = state_load_function(string);
        if (map_context.maps[i].load == NULL)
            throw_map_error(ERROR_MISSING);

        json_iterator_increment(it);
    }

    map_context.current_map = NULL;
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
