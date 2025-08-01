#include "internal.h"
#include "../state.h"
#include "../api.h"
#include <json.h>
#include <windows.h>
#include <stb_image.h>

typedef void (*MapLoadFuncPtr)(GlobalApi*);

typedef struct {
    u32 color;
    u32 top_tex;
    union {
        u32 tex;
        u32 side_tex;
    };
    bool is_wall;
} TileColor;

typedef struct {
    TileColor* colors;
    i32 num_colors;
} Palette;

typedef struct {
    // aabb in map's data
    i32 x, y, w, h;
    const char* type;
} Room;

typedef struct {
    // color data
    u32* data;
    Room* rooms;
    i32 num_rooms;
} Map;

typedef struct {

    const char** names;
    i32 num_names;

    // load as needed
    JsonObject* json;
    
    // error handling
    const char* current_map;

} MapContext;

extern GameContext game_context;
static MapContext map_context;

typedef enum {
    ERROR_MISSING,
    ERROR_INVALID_TYPE,
    ERROR_INVALID_VALUE,
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
        case ERROR_INVALID_VALUE:
            message = "wrong value";
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
    const char** names;
    const char* name;
    i32 num_names;

    num_names = json_object_length(json);
    names = st_malloc(num_names * sizeof(char*));
    for (i32 i = 0; i < num_names; i++) {
        member = json_iterator_get(it);
        if (member == NULL)
            throw_map_error(ERROR_GENERIC);

        name = json_member_key(member);
        if (name == NULL)
            throw_map_error(ERROR_GENERIC);

        names[i] = name;

        json_iterator_increment(it);
    }

    json_iterator_destroy(it);

    map_context.names = names;
    map_context.num_names = num_names;
    map_context.json = json;
}

static const char* load_string(JsonObject* object, const char* string)
{
    JsonValue* value;

    value = json_get_value(object, string);
    if (value == NULL)
        throw_map_error(ERROR_MISSING);
    if (json_get_type(value) != JTYPE_STRING)
        throw_map_error(ERROR_INVALID_TYPE);

    return json_get_string(value);
}

static void load_palette_color(JsonObject* object, TileColor* tile)
{
    const char* string = load_string(object, "color");
    if (strlen(string) != 6)
        throw_map_error(ERROR_INVALID_VALUE);
    tile->color = strtol(string, NULL, 16);
}

static void load_palette_type(JsonObject* object, TileColor* tile)
{
    const char* string = load_string(object, "type");
    if (strcmp(string, "tile") == 0)
        tile->is_wall = false;
    else if (strcmp(string, "wall") == 0)
        tile->is_wall = true;
    else
        throw_map_error(ERROR_INVALID_VALUE);
}


static void load_palette_textures(JsonObject* object, TileColor* tile)
{
    const char* string;
    if (tile->is_wall) {
        string = load_string(object, "side_tex");
        tile->side_tex = texture_get_id(string);
        string = load_string(object, "top_tex");
        tile->top_tex = texture_get_id(string);
    } else {
        string = load_string(object, "tex");
        tile->tex = texture_get_id(string);
    }
}

static Palette* palette_create(JsonObject* root)
{
    JsonObject* object;
    JsonValue* value;
    JsonIterator* it;
    JsonMember* member;
    Palette* palette;
    TileColor* colors;
    i32 num_colors;

    value = json_get_value(root, "palette");
    if (value == NULL)
        throw_map_error(ERROR_MISSING);
    if (json_get_type(value) != JTYPE_OBJECT)
        throw_map_error(ERROR_INVALID_TYPE);

    object = json_get_object(value);

    num_colors = json_object_length(object);
    colors = st_malloc(num_colors * sizeof(TileColor));

    it = json_iterator_create(object);
    for (i32 i = 0; i < num_colors; i++) {
        member = json_iterator_get(it);
        
        value = json_member_value(member);
        if (json_get_type(value) != JTYPE_OBJECT)
            throw_map_error(ERROR_INVALID_TYPE);

        object = json_get_object(value);
        
        load_palette_color(object, &colors[i]);
        load_palette_type(object, &colors[i]);
        load_palette_textures(object, &colors[i]);

        json_iterator_increment(it);
    }
    json_iterator_destroy(it);

    palette = st_malloc(sizeof(Palette));
    palette->colors = colors;
    palette->num_colors = num_colors;

    return palette;
}

static void palette_destroy(Palette* palette)
{
    st_free(palette->colors);
    st_free(palette); 
}

static void load_map_location(JsonObject* object, Room* room)
{
}

static Map* map_create(JsonObject* root, const char* path, Palette* palette)
{
    JsonObject* object;
    JsonValue* value;
    JsonIterator* it;
    JsonMember* member;
    Map* map;
    Room* rooms;
    Room* spawn_room = NULL;
    i32 x, y, n, i, j, idx;
    i32 num_rooms;
    u32* map_data;
    unsigned char* raw_data;

    value = json_get_value(root, "rooms");
    if (value == NULL)
        throw_map_error(ERROR_MISSING);
    if (json_get_type(value) != JTYPE_OBJECT)
        throw_map_error(ERROR_INVALID_TYPE);

    object = json_get_object(value);

    num_rooms = json_object_length(object);
    rooms = st_malloc(num_rooms * sizeof(Room));

    raw_data = stbi_load(path, &x, &y, &n, 3);
    if (raw_data == NULL)
        log_write(FATAL, "Could not load map data for %s", path);
    map_data = st_malloc(x * y * sizeof(u32));
    for (i = 0; i < y; i++) {
        for (j = 0; j < x; j++) {
            idx = (y-i-1) * x + j;
            map_data[i*x+j] = raw_data[3*idx+2] + (raw_data[3*idx+1]<<8) + (raw_data[3*idx]<<16);
        }
    }
    stbi_image_free(raw_data);

    it = json_iterator_create(object);
    for (i = 0; i < num_rooms; i++) {
        member = json_iterator_get(it);

        value = json_member_value(member);
        if (json_get_type(value) != JTYPE_OBJECT)
            throw_map_error(ERROR_MISSING);

        object = json_get_object(value);

        load_map_location(object, &rooms[i]);
        rooms[i].type = load_string(object, "type");
        //load_map_create(object, &rooms[i]);

        if (strcmp(rooms[i].type, "spawn") == 0) {
            if (spawn_room != NULL)
                throw_map_error(ERROR_GENERIC);
            spawn_room = &rooms[i];
        }

        json_iterator_increment(it);
    }
    json_iterator_destroy(it);

    if (spawn_room == NULL)
        throw_map_error(ERROR_GENERIC);

    map = st_malloc(sizeof(Map));
    map->rooms = rooms;
    map->num_rooms = num_rooms;
    map->data = map_data;

    return map;
}

static void map_destroy(Map* map)
{
    st_free(map->data);
    st_free(map->rooms);
    st_free(map);
}

static void generate_map(i32 id)
{
    JsonValue* value;
    JsonObject* object;
    Palette* palette;
    Map* map;
    const char* path;

    value = json_get_value(map_context.json, map_context.names[id]);
    if (value == NULL)
        throw_map_error(ERROR_MISSING);
    if (json_get_type(value) != JTYPE_OBJECT)
        throw_map_error(ERROR_INVALID_TYPE);

    object = json_get_object(value);

    value = json_get_value(object, "path");
    if (value == NULL)
        throw_map_error(ERROR_MISSING);
    if (json_get_type(value) != JTYPE_STRING)
        throw_map_error(ERROR_INVALID_TYPE);

    path = json_get_string(value);

    palette = palette_create(object);
    map = map_create(object, path, palette);

    palette_destroy(palette);
    map_destroy(map);
}

i32 map_get_id(const char* name)
{
    int l, r, m, a;
    l = 0;
    r = map_context.num_names - 1;
    while (l <= r) {
        m = l + (r - l) / 2;
        a = strcmp(name, map_context.names[m]);
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
    if (id == -1) {
        log_write(WARNING, "Tried to load map with id of -1");
        return;
    }

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

    generate_map(id);
}

void map_cleanup(void)
{
    st_free(map_context.names);
    json_object_destroy(map_context.json);
}
