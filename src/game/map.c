#include "internal.h"
#include "../state.h"
#include "../api.h"
#include <json.h>
#include <windows.h>
#include <stb_image.h>

#define DEFAULT_WALL_HEIGHT 1.5f

typedef void (*RoomCreateFuncPtr)(GlobalApi*, i32, i32);
typedef void (*TileCreateFuncPtr)(GlobalApi*, Tile*);

typedef struct {
    TileCreateFuncPtr create;
    TileCollideFuncPtr collide;
    u32 color;
    u32 top_tex;
    f32 height;
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
    i32 u1, v1, u2, v2;
    // relative spawn coordinates if spawn
    i32 spawn_x, spawn_z;
    const char* name;
    const char* type;
    RoomCreateFuncPtr create;
} Room;

typedef struct {
    i32 width, height;
    u32* data;
    Room* rooms;
    i32 num_rooms;
} RoomSet;

typedef struct {
    i32 x;
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
    // errors that im too lazy to implement right now
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
            message = "shit";
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

static const char* get_string_value(JsonObject* object, const char* string)
{
    JsonValue* value = json_get_value(object, string);
    if (value == NULL)
        throw_map_error(ERROR_MISSING);
    if (json_get_type(value) != JTYPE_STRING)
        throw_map_error(ERROR_INVALID_TYPE);
    return json_get_string(value);
}

static void load_palette_color(JsonObject* object, TileColor* tile)
{
    const char* string = get_string_value(object, "color");
    if (strlen(string) != 6)
        throw_map_error(ERROR_INVALID_VALUE);
    tile->color = strtol(string, NULL, 16);
}

static void load_palette_type(JsonObject* object, TileColor* tile)
{
    const char* string = get_string_value(object, "type");
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
        string = get_string_value(object, "side_tex");
        tile->side_tex = texture_get_id(string);
        string = get_string_value(object, "top_tex");
        tile->top_tex = texture_get_id(string);
    } else {
        string = get_string_value(object, "tex");
        tile->tex = texture_get_id(string);
    }
}

static void load_palette_create(JsonObject* object, TileColor* tile)
{
    JsonValue* value;
    const char* string;
    tile->create = NULL;

    value = json_get_value(object, "create");
    if (value == NULL)
        return;
    if (json_get_type(value) != JTYPE_STRING)
        throw_map_error(ERROR_INVALID_TYPE);

    string = json_get_string(value);
    tile->create = state_load_function(string);
    if (tile->create == NULL)
        throw_map_error(ERROR_MISSING);
}

static void load_palette_collide(JsonObject* object, TileColor* tile)
{
    JsonValue* value;
    const char* string;
    tile->collide = NULL;

    value = json_get_value(object, "collide");
    if (value == NULL)
        return;
    if (json_get_type(value) != JTYPE_STRING)
        throw_map_error(ERROR_INVALID_TYPE);

    string = json_get_string(value);
    tile->collide = state_load_function(string);
    if (tile->collide == NULL)
        throw_map_error(ERROR_MISSING);
}

static void load_palette_height(JsonObject* object, TileColor* tile)
{
    JsonValue* value;

    if (!tile->is_wall)
        return;
    
    tile->height = DEFAULT_WALL_HEIGHT;
    value = json_get_value(object, "height");
    if (value == NULL)
        return;

    if (json_get_type(value) == JTYPE_INT)
        tile->height = (f32)json_get_int(value);
    else if (json_get_type(value) == JTYPE_FLOAT)
        tile->height = json_get_float(value);
    else
        throw_map_error(ERROR_INVALID_TYPE);
}

static Palette* palette_create(JsonObject* root)
{
    JsonObject* object;
    JsonValue* value;
    JsonIterator* it;
    JsonMember* member;
    Palette* palette;
    TileColor* colors;
    TileColor* color;
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
        color = &colors[i];
        member = json_iterator_get(it);
        
        value = json_member_value(member);
        if (json_get_type(value) != JTYPE_OBJECT)
            throw_map_error(ERROR_INVALID_TYPE);

        object = json_get_object(value);
        
        load_palette_color(object, color);
        load_palette_type(object, color);
        load_palette_textures(object, color);
        load_palette_create(object, color);
        load_palette_collide(object, color);
        load_palette_height(object, color);

        json_iterator_increment(it);
    }
    json_iterator_destroy(it);

    palette = st_malloc(sizeof(Palette));
    palette->colors = colors;
    palette->num_colors = num_colors;

    return palette;
}

static TileColor* palette_get(Palette* palette, u32 color)
{
    for (i32 i = 0; i < palette->num_colors; i++)
        if (palette->colors[i].color == color)
            return &palette->colors[i];
    return NULL;
}

static void palette_destroy(Palette* palette)
{
    st_free(palette->colors);
    st_free(palette); 
}

static i32 get_int_value(JsonValue* value)
{
    if (value == NULL)
        throw_map_error(ERROR_MISSING);
    if (json_get_type(value) != JTYPE_INT)
        throw_map_error(ERROR_INVALID_TYPE);
    return json_get_int(value);
}

static RoomSet* room_set_create(JsonObject* root, const char* path)
{
    JsonObject* object;
    JsonValue* value;
    JsonIterator* it;
    JsonMember* member;
    JsonArray* array;
    i32 array_length;
    RoomSet* room_set;
    Room* rooms;
    Room* room;
    i32 num_rooms;
    Room* spawn_room = NULL;
    i32 x, y, n, i, j, idx;
    u32* room_set_data;
    unsigned char* raw_data;
    const char* string;

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
    room_set_data = st_malloc(x * y * sizeof(u32));
    for (i = 0; i < y; i++) {
        for (j = 0; j < x; j++) {
            idx = i * x + j;
            room_set_data[idx] = raw_data[3*idx+2] + (raw_data[3*idx+1]<<8) + (raw_data[3*idx]<<16);
        }
    }
    stbi_image_free(raw_data);

    it = json_iterator_create(object);
    for (i = 0; i < num_rooms; i++) {
        room = &rooms[i];
        member = json_iterator_get(it);

        room->name = json_member_key(member);

        value = json_member_value(member);
        if (json_get_type(value) != JTYPE_OBJECT)
            throw_map_error(ERROR_MISSING);

        object = json_get_object(value);

        value = json_get_value(object, "bounding_box");
        if (value == NULL)
            throw_map_error(ERROR_MISSING);
        if (json_get_type(value) != JTYPE_ARRAY)
            throw_map_error(ERROR_INVALID_TYPE);

        array = json_get_array(value);
        array_length = json_array_length(array);
        if (array_length != 4)
            throw_map_error(ERROR_GENERIC);
        
        room->u1 = get_int_value(json_array_get(array, 0));
        room->v1 = get_int_value(json_array_get(array, 1));
        room->u2 = get_int_value(json_array_get(array, 2));
        room->v2 = get_int_value(json_array_get(array, 3));

        string = get_string_value(object, "create");
        room->create = state_load_function(string);
        if (room->create == NULL)
            throw_map_error(ERROR_GENERIC);
        
        room->type = get_string_value(object, "type");
        if (strcmp(room->type, "spawn") != 0)
            goto increment;

        if (spawn_room != NULL)
            throw_map_error(ERROR_GENERIC);
        spawn_room = room;
        value = json_get_value(object, "spawn_point");
        if (value == NULL)
            throw_map_error(ERROR_MISSING);
        if (json_get_type(value) != JTYPE_ARRAY)
            throw_map_error(ERROR_INVALID_TYPE);

        array = json_get_array(value);
        array_length = json_array_length(array);
        if (array_length != 2)
            throw_map_error(ERROR_GENERIC);

        room->spawn_x = get_int_value(json_array_get(array, 0));
        room->spawn_z = get_int_value(json_array_get(array, 1));


increment:
        json_iterator_increment(it);
    }
    json_iterator_destroy(it);

    if (spawn_room == NULL)
        throw_map_error(ERROR_GENERIC);

    room_set = st_malloc(sizeof(RoomSet));
    room_set->rooms = rooms;
    room_set->num_rooms = num_rooms;
    room_set->data = room_set_data;
    room_set->width = x;
    room_set->height = y;

    return room_set;
}

static Room* room_set_get(RoomSet* room_set, const char* type)
{
    for (i32 i = 0; i < room_set->num_rooms; i++)
        if (strcmp(room_set->rooms[i].type, type) == 0)
            return &room_set->rooms[i];
    return NULL;
}

static void room_set_destroy(RoomSet* room_set)
{
    st_free(room_set->data);
    st_free(room_set->rooms);
    st_free(room_set);
}

#define WHITE   0x000000
#define GRAY    0x808080
#define BLACK   0xFFFFFF

static void load_room(Palette* palette, RoomSet* room_set, Room* room, i32 origin_x, i32 origin_y)
{
    TileColor* tile_color;
    Tile* tile;
    Wall* wall;
    i32 idx;
    u32 color;
    vec2 position;
    for (i32 i = room->v1; i < room->v2; i++) {
        position.y = origin_y + i - room->v1;
        for (i32 j = room->u1; j < room->u2; j++) {
            position.x = origin_x + j - room->u1;
            idx = i * room_set->width + j;
            color = room_set->data[idx];
            if (color == WHITE || color == GRAY || color == BLACK)
                continue;
            tile_color = palette_get(palette, color);
            if (tile_color == NULL)
                throw_map_error(ERROR_GENERIC);
            if (tile_color->is_wall) {
                wall = wall_create(position, tile_color->height);
                wall->side_tex = tile_color->side_tex;
                wall->top_tex = tile_color->top_tex;
            } else {
                tile = tile_create(position);
                tile->tex = tile_color->tex;
                tile->collide = tile_color->collide;
                if (tile_color->create != NULL)
                    tile_color->create(&global_api, tile);
            }
        }
    }
}

static void generate_map(i32 id)
{
    JsonValue* value;
    JsonObject* object;
    Palette* palette;
    RoomSet* room_set;
    const char* path;
    i32 origin_x = 0, origin_y = 0;

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
    room_set = room_set_create(object, path);
    palette = palette_create(object);

    Room* spawn_room = room_set_get(room_set, "spawn");
    if (spawn_room == NULL)
        throw_map_error(ERROR_GENERIC);

    load_room(palette, room_set, spawn_room, origin_x, origin_y);
    game_set_player_position(vec2_create(origin_x+spawn_room->spawn_x+0.5, origin_y+spawn_room->spawn_z+0.5));
    spawn_room->create(&global_api, origin_x, origin_y);

    palette_destroy(palette);
    room_set_destroy(room_set);
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
