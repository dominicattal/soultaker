#include "../game.h"
#include "../event.h"
#include "../state.h"
#include "../api.h"
#include <json.h>
#include <math.h>
#include <string.h>
#include <stb_image.h>

// Think of the map like a tree with rooms as nodes.
// Spawn room is like the root of the tree, and there are
// branches the player may go down that may lead to an event
// or a boss room.

#define DEFAULT_WALL_HEIGHT 1.5f
#define MAP_MAX_WIDTH   10000
#define MAP_MAX_LENGTH  10000
#define WHITE   0xFFFFFF
#define GRAY    0x808080
#define BLACK   0x000000

// function called when room in generated. used to create game objects in that room
typedef void (*RoomCreateFuncPtr)(GameApi*);
// function called when player enters a room. i32 arg is the number of times that player entered
typedef void (*RoomEnterFuncPtr)(GameApi*, i32);
// functionc alled when player exits a room.
typedef void (*RoomExitFuncPtr)(GameApi*, i32);
// create data for map that persists while map is in memory
// this data can be queried with map_get_data()
typedef void* (*RoomsetInitFuncPtr)(GameApi*);
// last function called before map is destroyed, used to cleanup allocations
// in RoomsetInitFuncPtr. void* arg is the data
typedef void (*RoomsetCleanupFuncPtr)(GameApi*, void*);
// true if at end of branch, false otherwise
typedef bool (*RoomsetGenerateFuncPtr)(GameApi*, LocalMapGenerationSettings*);
// true if should create branch, false otherwise
typedef bool (*RoomsetBranchFuncPtr)(GameApi*, LocalMapGenerationSettings*);
// function called when a tile is made
typedef void (*TileCreateFuncPtr)(GameApi*, Tile*);

typedef struct {
    const char* name;
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
    i32 u1, v1, u2, v2;
    i32 origin_u, origin_v;
    i32 loc_u, loc_v;
    TileColor* default_tile;
} Alternate;

typedef struct Room {
    i32 u1, v1, u2, v2;
    const char* name;
    const char* type;
    RoomCreateFuncPtr create;
    RoomEnterFuncPtr enter;
    RoomExitFuncPtr exit;
    List* male_alternates;
    List* female_alternates;
} Room;

typedef struct {
    i32 width, length;
    u32* pixels;
    Room* rooms;
    i32 num_rooms;
    void* data;
    Palette* palette;
    RoomsetInitFuncPtr init;
    RoomsetGenerateFuncPtr generate;
    RoomsetBranchFuncPtr branch;
    RoomsetCleanupFuncPtr cleanup;
} Roomset;

typedef struct Map {
    // x is width, z is length
    i32 width, length;
    vec2 spawn_point;
    Roomset* roomset;
    MapNode* root;
    void** tilemap;
    MapNode** map_nodes;
    Quadmask* tile_mask;
    Quadmask* fog_mask;
    List* bosses;
    List* entities;
    List* tiles;
    List* walls;
    List* free_walls;
    List* projectiles;
    List* obstacles;
    List* parstacles;
    List* particles;
    List* parjicles;
    List* triggers;
    List* aoes;
    bool active;
} Map;

typedef struct MapNode {
    MapNode* parent;
    MapNode** children;
    Alternate* female_alternate;
    Room* room;
    List* male_alternates;
    i32 num_children;
    i32 origin_x, origin_z;
    i32 x1, x2, z1, z2;
    i32 orientation;
    i32 num_exits;
    i32 num_enters;
    bool visited;
    bool cleared;
} MapNode;

typedef struct {
    Quadmask* qm;
    Roomset* roomset;
} GlobalMapGenerationSettings;

typedef struct {

    const char** names;
    i32 num_names;
    JsonObject* json;

    // map generating / map events
    Map* current_map;
    MapNode* current_map_node;
    
    // error handling
    const char* current_map_name;

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
    const char* name = map_context.current_map_name;
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

static const char* get_string_value(JsonObject* object, const char* string)
{
    JsonValue* value = json_object_get_value(object, string);
    if (value == NULL)
        throw_map_error(ERROR_MISSING);
    if (json_value_get_type(value) != JTYPE_STRING)
        throw_map_error(ERROR_INVALID_TYPE);
    return json_value_get_string(value);
}

static JsonArray* get_array_value(JsonObject* object, const char* string)
{
    JsonValue* value = json_object_get_value(object, string);
    if (value == NULL)
        throw_map_error(ERROR_MISSING);
    if (json_value_get_type(value) != JTYPE_ARRAY)
        throw_map_error(ERROR_INVALID_TYPE);
    return json_value_get_array(value);
}

static void load_palette_color(JsonObject* object, TileColor* tile)
{
    const size_t HEX_LENGTH = 6;
    const char* string = get_string_value(object, "color");
    if (strlen(string) != HEX_LENGTH)
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

    value = json_object_get_value(object, "create");
    if (value == NULL)
        return;
    if (json_value_get_type(value) != JTYPE_STRING)
        throw_map_error(ERROR_INVALID_TYPE);

    string = json_value_get_string(value);
    tile->create = state_load_function(string);
    if (tile->create == NULL)
        throw_map_error(ERROR_MISSING);
}

static void load_palette_collide(JsonObject* object, TileColor* tile)
{
    JsonValue* value;
    const char* string;
    tile->collide = NULL;

    value = json_object_get_value(object, "collide");
    if (value == NULL)
        return;
    if (json_value_get_type(value) != JTYPE_STRING)
        throw_map_error(ERROR_INVALID_TYPE);

    string = json_value_get_string(value);
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
    value = json_object_get_value(object, "height");
    if (value == NULL)
        return;

    if (json_value_get_type(value) == JTYPE_INT)
        tile->height = (f32)json_value_get_int(value);
    else if (json_value_get_type(value) == JTYPE_FLOAT)
        tile->height = json_value_get_float(value);
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

    value = json_object_get_value(root, "palette");
    if (value == NULL)
        throw_map_error(ERROR_MISSING);
    if (json_value_get_type(value) != JTYPE_OBJECT)
        throw_map_error(ERROR_INVALID_TYPE);

    object = json_value_get_object(value);

    num_colors = json_object_length(object);
    colors = st_malloc(num_colors * sizeof(TileColor));

    it = json_iterator_create(object);
    for (i32 i = 0; i < num_colors; i++) {
        color = &colors[i];
        member = json_iterator_get(it);
        color->name = json_member_get_key(member);
        value = json_member_get_value(member);
        if (json_value_get_type(value) != JTYPE_OBJECT)
            throw_map_error(ERROR_INVALID_TYPE);

        object = json_value_get_object(value);
        
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

static TileColor* palette_get_from_name(Palette* palette, const char* name)
{
    i32 l, r, m, a;
    l = 0;
    r = palette->num_colors-1;
    while (l <= r) {
        m = l + (r - l) / 2;
        a = strcmp(name, palette->colors[m].name);
        if (a > 0)
            l = m + 1;
        else if (a < 0)
            r = m - 1;
        else
            return &palette->colors[m];
    }
    log_write(WARNING, "Could not get id for %s", name);
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
    if (json_value_get_type(value) != JTYPE_INT)
        throw_map_error(ERROR_INVALID_TYPE);
    return json_value_get_int(value);
}

static void parse_room_bounding_box(JsonObject* object, Room* room)
{
    JsonArray* array;
    i32 array_length;

    array = get_array_value(object, "bounding_box");
    array_length = json_array_length(array);
    if (array_length != 4)
        throw_map_error(ERROR_GENERIC);
    
    room->u1 = get_int_value(json_array_get(array, 0));
    room->v1 = get_int_value(json_array_get(array, 1));
    room->u2 = get_int_value(json_array_get(array, 2));
    room->v2 = get_int_value(json_array_get(array, 3));
}

static void parse_room_create(JsonObject* object, Room* room)
{
    JsonValue* value;
    room->create = NULL;
    value = json_object_get_value(object, "create");
    if (value == NULL) 
        return;
    if (json_value_get_type(value) != JTYPE_STRING)
        return;
    room->create = state_load_function(json_value_get_string(value));
}

static void parse_room_enter(JsonObject* object, Room* room)
{
    JsonValue* value;
    room->enter = NULL;
    value = json_object_get_value(object, "enter");
    if (value == NULL) 
        return;
    if (json_value_get_type(value) != JTYPE_STRING)
        return;
    room->enter = state_load_function(json_value_get_string(value));
}

static void parse_room_exit(JsonObject* object, Room* room)
{
    JsonValue* value;
    room->exit = NULL;
    value = json_object_get_value(object, "exit");
    if (value == NULL) 
        return;
    if (json_value_get_type(value) != JTYPE_STRING)
        return;
    room->exit = state_load_function(json_value_get_string(value));
}

static void parse_room_type(JsonObject* object, Room* room)
{
    room->type = get_string_value(object, "type");
}

static void parse_room_alternates(JsonObject* object, Palette* palette, Room* room)
{
    JsonValue* value;
    JsonArray* array;
    JsonArray* num_array;
    Alternate* alternate;
    const char* string;
    i32 num_array_length, array_length, i;

    room->male_alternates = list_create();
    room->female_alternates = list_create();
    value = json_object_get_value(object, "alternates");
    if (value == NULL)
        return;
    if (json_value_get_type(value) != JTYPE_ARRAY)
        throw_map_error(ERROR_GENERIC);
    array = json_value_get_array(value);
    array_length = json_array_length(array);

    for (i = 0; i < array_length; i++) {
        alternate = st_malloc(sizeof(Alternate));
        value = json_array_get(array, i);
        if (json_value_get_type(value) != JTYPE_OBJECT)
            throw_map_error(ERROR_INVALID_TYPE);
        object = json_value_get_object(value);

        num_array = get_array_value(object, "bounding_box");
        num_array_length = json_array_length(num_array);
        if (num_array_length != 4)
            throw_map_error(ERROR_GENERIC);

        alternate->u1 = get_int_value(json_array_get(num_array, 0));
        alternate->v1 = get_int_value(json_array_get(num_array, 1));
        alternate->u2 = get_int_value(json_array_get(num_array, 2));
        alternate->v2 = get_int_value(json_array_get(num_array, 3));

        num_array = get_array_value(object, "location");
        num_array_length = json_array_length(num_array);
        if (num_array_length != 2)
            throw_map_error(ERROR_GENERIC);

        alternate->loc_u = get_int_value(json_array_get(num_array, 0));
        alternate->loc_v = get_int_value(json_array_get(num_array, 1));

        num_array = get_array_value(object, "origin");
        num_array_length = json_array_length(num_array);
        if (num_array_length != 2)
            throw_map_error(ERROR_GENERIC);

        alternate->origin_u = get_int_value(json_array_get(num_array, 0));
        alternate->origin_v = get_int_value(json_array_get(num_array, 1));

        value = json_object_get_value(object, "default");
        if (value == NULL)
            alternate->default_tile = NULL;
        else {
            if (json_value_get_type(value) != JTYPE_STRING)
                throw_map_error(ERROR_INVALID_TYPE);
            string = json_value_get_string(value);
            alternate->default_tile = palette_get_from_name(palette, string);
            if (alternate->default_tile == NULL)
                throw_map_error(ERROR_GENERIC);
        }

        string = get_string_value(object, "type");
        if (strcmp(string, "male") == 0)
            list_append(room->male_alternates, alternate);
        else if (strcmp(string, "female") == 0)
            list_append(room->female_alternates, alternate);
        else
            throw_map_error(ERROR_GENERIC);
    }
}

typedef struct {
    List* list;
    Room* room;
    u32* data;
    i32 width;
    i32 length;
    const char* string;
    u32 color;
} VerifyRoomAltArgs;

static bool verify_room_alternate_list(VerifyRoomAltArgs* args)
{
    Room* room = args->room;
    List* list = args->list;
    i32 width = args->width;
    i32 length = args->length;
    const char* string = args->string;
    u32* data = args->data;
    u32 color = args->color;
    Alternate* alternate;
    Alternate* a1;
    Alternate* a2;
    i32 i, j, idx, u, v;
    i32 u1, v1, u2, v2;

    // check if alternate does not match data
    for (i = 0; i < list->length; i++) {
        alternate = list_get(list, i);
        u = alternate->loc_u;
        v = alternate->loc_v;
        idx = v * width + u;
        if (idx < 0 || idx >= width * length) {
            log_write(CRITICAL, "%s alternate location (%d %d) out of bounds", string, u, v);
            return false;
        }
        if (data[idx] != color) {
            log_write(CRITICAL, "%s alternate location (%d %d) not valid", string, u, v);
            return false;
        }
    }

    // check if data does not match alternate
    for (v = room->v1; v <= room->v2; v++) {
        for (u = room->u1; u <= room->u2; u++) {
            idx = v * width + u;
            if (data[idx] != color)
                continue;
            for (i = 0; i < list->length; i++) {
                alternate = list_get(list, i);
                if (alternate->loc_u != u)
                    continue;
                if (alternate->loc_v != v)
                    continue;
                goto found;
            }
            log_write(CRITICAL, "%s alternate not found for %d %d", string, u, v);
            return false;
            found:
        }
    }

    // check duplicate alternates
    for (i = 0; i < list->length; i++) {
        a1 = list_get(list, i);
        for (j = 0; j < i; j++) {
            a2 = list_get(list, j);
            if (a1->loc_u != a2->loc_u)
                continue;
            if (a1->loc_v != a2->loc_v)
                continue;
            log_write(CRITICAL, "duplicate %s alternate at (%d %d)", string, a1->loc_u, a1->loc_v);
            return false;
        }
    }

    // check if alternate bounding box contains gray or black
    for (i = 0; i < list->length; i++) {
        alternate = list_get(list, i);
        u1 = alternate->u1;
        u2 = alternate->u2;
        v1 = alternate->v1;
        v2 = alternate->v2;
        for (v = v1; v <= v2; v++) {
            for (u = u1; u <= u2; u++) {
                idx = v * width + u;
                if (data[idx] == GRAY || data[idx] == BLACK) {
                    log_write(CRITICAL, "%s alternate bounding box contains invalid color", string);
                    return false;
                }
            }
        }
    }

    return true;
}

static bool verify_room_alternates(Room* room, u32* data, i32 width, i32 length)
{
    VerifyRoomAltArgs args;
    args.room = room;
    args.data = data;
    args.width = width;
    args.length = length;

    args.list = room->female_alternates;
    args.string = "female";
    args.color = GRAY;
    if (!verify_room_alternate_list(&args))
        return false;

    args.list = room->male_alternates;
    args.string = "male";
    args.color = BLACK;
    if (!verify_room_alternate_list(&args))
        return false;

    return true;
}

static Roomset* roomset_create(JsonObject* root, const char* path, Palette* palette)
{
    JsonObject* object;
    JsonValue* value;
    JsonIterator* it;
    JsonMember* member;
    Roomset* roomset;
    Room* rooms;
    Room* room;
    i32 num_rooms;
    i32 x, y, n, i, j, idx;
    u32* pixels;
    RoomsetInitFuncPtr init;
    RoomsetGenerateFuncPtr generate;
    RoomsetBranchFuncPtr branch;
    RoomsetCleanupFuncPtr cleanup;
    unsigned char* raw_data;

    generate = state_load_function(get_string_value(root, "generate"));
    if (generate == NULL)
        throw_map_error(ERROR_MISSING);

    branch = state_load_function(get_string_value(root, "branch"));
    if (branch == NULL)
        throw_map_error(ERROR_MISSING);

    init = state_load_function(get_string_value(root, "init"));
    if (init == NULL)
        throw_map_error(ERROR_MISSING);

    cleanup = state_load_function(get_string_value(root, "cleanup"));
    if (cleanup == NULL)
        throw_map_error(ERROR_MISSING);

    value = json_object_get_value(root, "rooms");
    if (value == NULL)
        throw_map_error(ERROR_MISSING);
    if (json_value_get_type(value) != JTYPE_OBJECT)
        throw_map_error(ERROR_INVALID_TYPE);

    object = json_value_get_object(value);

    num_rooms = json_object_length(object);
    rooms = st_malloc(num_rooms * sizeof(Room));

    raw_data = stbi_load(path, &x, &y, &n, 3);
    if (raw_data == NULL)
        log_write(FATAL, "Could not load map data for %s", path);
    pixels = st_malloc(x * y * sizeof(u32));
    for (i = 0; i < y; i++) {
        for (j = 0; j < x; j++) {
            idx = i * x + j;
            pixels[idx] = raw_data[3*idx+2] + (raw_data[3*idx+1]<<8) + (raw_data[3*idx]<<16);
        }
    }
    stbi_image_free(raw_data);

    it = json_iterator_create(object);
    for (i = 0; i < num_rooms; i++) {
        room = &rooms[i];
        member = json_iterator_get(it);

        room->name = json_member_get_key(member);

        value = json_member_get_value(member);
        if (json_value_get_type(value) != JTYPE_OBJECT)
            throw_map_error(ERROR_MISSING);

        object = json_value_get_object(value);

        parse_room_bounding_box(object, room);
        parse_room_create(object, room);
        parse_room_enter(object, room);
        parse_room_exit(object, room);
        parse_room_type(object, room);
        parse_room_alternates(object, palette, room);

        if (!verify_room_alternates(room, pixels, x, y))
            throw_map_error(ERROR_MISSING);

        json_iterator_increment(it);
    }
    json_iterator_destroy(it);

    roomset = st_malloc(sizeof(Roomset));
    roomset->rooms = rooms;
    roomset->num_rooms = num_rooms;
    roomset->pixels = pixels;
    roomset->width = x;
    roomset->length = y;
    roomset->palette = palette;
    roomset->generate = generate;
    roomset->branch = branch;
    roomset->init = init;
    roomset->cleanup = cleanup;
    roomset->data = roomset->init(&game_api);

    return roomset;
}

static u32 roomset_get_color(Roomset* roomset, i32 u, i32 v)
{
    return roomset->pixels[v * roomset->width + u];
}

static List* roomset_get_rooms(Roomset* roomset, const char* type)
{
    List* rooms = list_create();
    Room* room;
    i32 i;
    for (i = 0; i < roomset->num_rooms; i++) {
        room = &roomset->rooms[i];
        if (strcmp(room->type, type) == 0)
            list_append(rooms, room);
    }
    return rooms;
}

static void roomset_destroy(Roomset* roomset)
{
    List* list;
    i32 i, j;
    roomset->cleanup(&game_api, roomset->data);
    for (i = 0; i < roomset->num_rooms; i++) {
        list = roomset->rooms[i].male_alternates;
        for (j = 0; j < list->length; j++)
            st_free(list_get(list, j));
        list_destroy(list);
        list = roomset->rooms[i].female_alternates;
        for (j = 0; j < list->length; j++)
            st_free(list_get(list, j));
        list_destroy(list);
    }
    st_free(roomset->rooms);
    st_free(roomset->pixels);
    st_free(roomset);
}

typedef struct {
    Quadmask* qm;
    Roomset* roomset;
    Room* room;
    Alternate* alternate;
    i32 origin_x;
    i32 origin_z;
    i32 orientation;
} PreloadArgs;

static bool color_is_preset(u32 color)
{
    return color == WHITE || color == GRAY || color == BLACK;
}

typedef enum {
    ROTATION_R0 = 0,
    ROTATION_R1,
    ROTATION_R2,
    ROTATION_R3,
    // mirrored rotations
    ROTATION_MR0,
    ROTATION_MR1,
    ROTATION_MR2,
    ROTATION_MR3,
    NUM_ORIENTATIONS
} Orientation;

static i32 calculate_room_dx(Room* room, Orientation orientation, i32 u, i32 v)
{
    i32 du, dv, w, l;
    w = room->u2 - room->u1 + 1;
    l = room->v2 - room->v1 + 1;
    du = u - room->u1;
    dv = v - room->v1;
    switch (orientation) {
        case ROTATION_R0:
        // fall through
        case ROTATION_MR0:
            return du;
        case ROTATION_R1:
        // fall through
        case ROTATION_MR1:
            return l - dv;
        case ROTATION_R2:
        // fall through
        case ROTATION_MR2:
            return w - du;
        case ROTATION_R3:
        // fall through
        case ROTATION_MR3:
            return dv;
        default:
            break;
    }
    return 0;
}

static i32 calculate_room_dz(Room* room, Orientation orientation, i32 u, i32 v)
{
    i32 du, dv, w, l;
    w = room->u2 - room->u1 + 1;
    l = room->v2 - room->v1 + 1;
    du = u - room->u1;
    dv = v - room->v1;
    switch (orientation) {
        case ROTATION_R1:
        // fall through
        case ROTATION_MR1:
            return du;
        case ROTATION_R2:
        // fall through
        case ROTATION_MR0:
            return l - dv;
        case ROTATION_R3:
        // fall through
        case ROTATION_MR3:
            return w - du;
        case ROTATION_R0:
        // fall through
        case ROTATION_MR2:
            return dv;
        default:
            break;
    }
    return 0;
}

static f64 calculate_room_dxf(Room* room, Orientation orientation, f64 u, f64 v)
{
    f64 du, dv, w, l;
    w = room->u2 - room->u1 + 1;
    l = room->v2 - room->v1 + 1;
    du = u - room->u1;
    dv = v - room->v1;
    switch (orientation) {
        case ROTATION_R0:
        // fall through
        case ROTATION_MR0:
            return du;
        case ROTATION_R1:
        // fall through
        case ROTATION_MR1:
            return l - dv + 1;
        case ROTATION_R2:
        // fall through
        case ROTATION_MR2:
            return w - du + 1;
        case ROTATION_R3:
        // fall through
        case ROTATION_MR3:
            return dv;
        default:
            break;
    }
    return 0;
}

static f64 calculate_room_dzf(Room* room, Orientation orientation, f64 u, f64 v)
{
    f64 du, dv, w, l;
    w = room->u2 - room->u1 + 1;
    l = room->v2 - room->v1 + 1;
    du = u - room->u1;
    dv = v - room->v1;
    switch (orientation) {
        case ROTATION_R1:
        // fall through
        case ROTATION_MR1:
            return du;
        case ROTATION_R2:
        // fall through
        case ROTATION_MR0:
            return l - dv + 1;
        case ROTATION_R3:
        // fall through
        case ROTATION_MR3:
            return w - du + 1;
        case ROTATION_R0:
        // fall through
        case ROTATION_MR2:
            return dv;
        default:
            break;
    }
    return 0;
}

static i32 calculate_alternate_dx(Room* room, Alternate* alternate, i32 orientation, i32 u, i32 v)
{
    i32 ru, rv;
    ru = u + alternate->loc_u - alternate->origin_u;
    rv = v + alternate->loc_v - alternate->origin_v;
    return calculate_room_dx(room, orientation, ru, rv);
}

static i32 calculate_alternate_dz(Room* room, Alternate* alternate, i32 orientation, i32 u, i32 v)
{
    i32 ru, rv;
    ru = u + alternate->loc_u - alternate->origin_u;
    rv = v + alternate->loc_v - alternate->origin_v;
    return calculate_room_dz(room, orientation, ru, rv);
}

static bool can_preload_room(PreloadArgs* args)
{
    Quadmask* qm = args->qm;
    Roomset* roomset = args->roomset;
    Room* room = args->room;
    i32 origin_x = args->origin_x;
    i32 origin_z = args->origin_z;
    i32 orientation = args->orientation;
    u32 color;
    i32 u, v, dx, dz, map_x, map_z;
    for (v = room->v1; v <= room->v2; v++) {
        for (u = room->u1; u <= room->u2; u++) {
            dx = calculate_room_dx(room, orientation, u, v);
            dz = calculate_room_dz(room, orientation, u, v);
            color = roomset_get_color(roomset, u, v);
            map_x = origin_x + dx;
            map_z = origin_z + dz;
            if (!quadmask_in_bounds(qm, map_x, map_z))
                return false;
            if (color_is_preset(color))
                continue;
            if (!quadmask_isset(qm, map_x, map_z))
                continue;
            return false;
        }
    }
    return true;
}

static bool can_preload_room_alternate(PreloadArgs* args)
{
    Quadmask* qm = args->qm;
    Roomset* roomset = args->roomset;
    Room* room = args->room;
    Alternate* alternate = args->alternate;
    i32 origin_x = args->origin_x;
    i32 origin_z = args->origin_z;
    i32 orientation = args->orientation;
    u32 room_color;
    i32 u, v, dx, dz, map_x, map_z;
    bool room_in_bounds;
    for (v = alternate->v1; v <= alternate->v2; v++) {
        for (u = alternate->u1; u <= alternate->u2; u++) {
            dx = calculate_alternate_dx(room, alternate, orientation, u, v);
            dz = calculate_alternate_dz(room, alternate, orientation, u, v);
            map_x = origin_x + dx;
            map_z = origin_z + dz;
            if (!quadmask_in_bounds(qm, map_x, map_z))
                return false;
            if (!quadmask_isset(qm, map_x, map_z))
                continue;
            room_in_bounds = dx >= 0 && dx <= room->u2-room->u1 && dz >= 0 && dz <= room->v2-room->v1;
            if (!room_in_bounds)
                return false;
            room_color = roomset_get_color(roomset, u, v);
            if (room_color != WHITE)
                continue;
            return false;
        }
    }
    return true;
}

static void preload_room(PreloadArgs* args)
{
    Quadmask* qm = args->qm;
    Roomset* roomset = args->roomset;
    Room* room = args->room;
    i32 origin_x = args->origin_x;
    i32 origin_z = args->origin_z;
    i32 orientation = args->orientation;
    u32 color;
    i32 u, v, dx, dz, map_x, map_z;
    for (v = room->v1; v <= room->v2; v++) {
        for (u = room->u1; u <= room->u2; u++) {
            dx = calculate_room_dx(room, orientation, u, v);
            dz = calculate_room_dz(room, orientation, u, v);
            color = roomset_get_color(roomset, u, v);
            map_x = origin_x + dx;
            map_z = origin_z + dz;
            if (color_is_preset(color))
                continue;
            quadmask_set(qm, map_x, map_z);
        }
    }
}

static Quadmask* preload_room_alternate(PreloadArgs* args)
{
    Quadmask* qm = args->qm;
    Roomset* roomset = args->roomset;
    Room* room = args->room;
    Alternate* alternate = args->alternate;
    i32 origin_x = args->origin_x;
    i32 origin_z = args->origin_z;
    i32 orientation = args->orientation;
    Quadmask* alt_qm;
    u32 color;
    i32 u, v, dx, dz, map_x, map_z;
    alt_qm = quadmask_create( alternate->v2-alternate->v1+1, alternate->u2-alternate->u1+1);
    for (v = alternate->v1; v <= alternate->v2; v++) {
        for (u = alternate->u1; u <= alternate->u2; u++) {
            dx = calculate_alternate_dx(room, alternate, orientation, u, v);
            dz = calculate_alternate_dz(room, alternate, orientation, u, v);
            color = roomset_get_color(roomset, u, v);
            map_x = origin_x + dx;
            map_z = origin_z + dz;
            if (quadmask_isset(qm, map_x, map_z))
                quadmask_set(alt_qm, v-alternate->v1, u-alternate->u1);
            //quadmask_unset(qm, map_x, map_z);
            if (color_is_preset(color))
                continue;
            quadmask_set(qm, map_x, map_z);
        }
    }
    return alt_qm;
}

static void unpreload_room(PreloadArgs* args)
{
    Quadmask* qm = args->qm;
    Roomset* roomset = args->roomset;
    Room* room = args->room;
    i32 origin_x = args->origin_x;
    i32 origin_z = args->origin_z;
    i32 orientation = args->orientation;
    u32 color;
    i32 u, v, dx, dz, map_x, map_z;
    for (v = room->v1; v <= room->v2; v++) {
        for (u = room->u1; u <= room->u2; u++) {
            dx = calculate_room_dx(room, orientation, u, v);
            dz = calculate_room_dz(room, orientation, u, v);
            color = roomset_get_color(roomset, u, v);
            map_x = origin_x + dx;
            map_z = origin_z + dz;
            if (color_is_preset(color))
                continue;
            quadmask_unset(qm, map_x, map_z);
        }
    }
}

static void unpreload_room_alternate(PreloadArgs* args, Quadmask* alt_qm)
{
    Quadmask* qm = args->qm;
    Room* room = args->room;
    Alternate* alternate = args->alternate;
    i32 origin_x = args->origin_x;
    i32 origin_z = args->origin_z;
    i32 orientation = args->orientation;
    i32 u, v, dx, dz, map_x, map_z;
    for (v = alternate->v1; v <= alternate->v2; v++) {
        for (u = alternate->u1; u <= alternate->u2; u++) {
            dx = calculate_alternate_dx(room, alternate, orientation, u, v);
            dz = calculate_alternate_dz(room, alternate, orientation, u, v);
            map_x = origin_x + dx;
            map_z = origin_z + dz;
            quadmask_unset(qm, map_x, map_z);
            if (quadmask_isset(alt_qm, v-alternate->v1, u-alternate->u1))
                quadmask_set(qm, map_x, map_z);
        }
    }
}

static MapNode* map_node_create(void)
{
    MapNode* node = st_malloc(sizeof(MapNode));
    node->parent = NULL;
    node->children = NULL;
    node->num_children = 0;
    node->room = NULL;
    node->female_alternate = NULL;
    node->male_alternates = list_create();
    node->visited = false;
    node->cleared = false;
    node->num_exits = 0;
    node->num_enters = 0;
    node->x1 = node->x2 = 0;
    node->z1 = node->z2 = 0;
    return node;
}

static void map_node_destroy(MapNode* node)
{
    for (i32 i = 0; i < node->num_children; i++)
        map_node_destroy(node->children[i]);
    list_destroy(node->male_alternates);
    st_free(node->children);
    st_free(node);
}

static void map_node_attach(MapNode* parent, MapNode* child)
{
    if (parent->children == NULL)
        parent->children = st_malloc(sizeof(MapNode));
    else
        parent->children = st_realloc(parent->children, (parent->num_children+1) * sizeof(MapNode));
    parent->children[parent->num_children++] = child;
    child->parent = parent;
}

static void map_node_detach(MapNode* parent, MapNode* child)
{
    for (i32 i = 0; i < parent->num_children; i++) {
        if (parent->children[i] == child) {
            parent->children[i] = parent->children[--parent->num_children];
            child->parent = NULL;
            return;
        }
    }
}

// returns true if successfully generated a room, false otherwise
static bool pregenerate_map_helper(GlobalMapGenerationSettings* global_settings, LocalMapGenerationSettings local_settings, MapNode* parent)
{
    PreloadArgs args;
    Quadmask* qm = global_settings->qm;
    Quadmask* fem_qm = NULL;
    Quadmask* male_qm = NULL;
    Roomset* roomset = global_settings->roomset;
    i32 male_x = local_settings.male_x;
    i32 male_z = local_settings.male_z;
    const char* current_room_type;
    List* rooms;
    List* female_alternates;
    List* male_alternates;
    Alternate* female_alternate;
    Alternate* male_alternate;
    MapNode* child;
    Room* room;
    i32 initial_orientation;
    i32 orientation_iter;
    i32 orientation;
    i32 origin_x, origin_z;
    i32 room_idx, fem_idx, male_idx;
    i32 u, v, dx, dz;
    i32 list_idx;

    args.qm = qm;
    args.roomset = roomset;

    if (roomset->generate(&game_api, &local_settings))
        return true;
    if (local_settings.no_path)
        return local_settings.succeed_even_if_no_path;

    current_room_type = local_settings.current_room_type;
    local_settings.num_rooms_loaded = 0;

    rooms = roomset_get_rooms(roomset, current_room_type);
    list_shuffle(rooms);
    for (room_idx = 0; room_idx < rooms->length; room_idx++) {
        room = list_get(rooms, room_idx);
        args.room = room;
        female_alternates = list_copy(room->female_alternates);
        list_shuffle(female_alternates);
        for (fem_idx = 0; fem_idx < female_alternates->length; fem_idx++) {
            female_alternate = list_get(female_alternates, fem_idx);
            args.alternate = female_alternate;
            initial_orientation = rand() % NUM_ORIENTATIONS;
            for (orientation_iter = 0; orientation_iter < NUM_ORIENTATIONS; orientation_iter++) {
                orientation = (initial_orientation + orientation_iter) % NUM_ORIENTATIONS;
                u = female_alternate->loc_u;
                v = female_alternate->loc_v;
                dx = calculate_room_dx(room, orientation, u, v);
                dz = calculate_room_dz(room, orientation, u, v);
                origin_x = male_x - dx;
                origin_z = male_z - dz;
                args.origin_x = origin_x;
                args.origin_z = origin_z;
                args.orientation = orientation;
                if (!can_preload_room(&args))
                    continue;
                if (!can_preload_room_alternate(&args))
                    continue;
                preload_room(&args);
                fem_qm = preload_room_alternate(&args);
                local_settings.num_rooms_left--;
                child = map_node_create();
                child->room = room;
                child->female_alternate = female_alternate;
                child->origin_x = origin_x;
                child->origin_z = origin_z;
                child->x1 = child->x2 = origin_x;
                child->z1 = child->z2 = origin_z;
                child->orientation = orientation;
                map_node_attach(parent, child);
                male_alternates = list_copy(room->male_alternates);
                if (local_settings.create_no_path)
                    goto no_path;
                list_shuffle(male_alternates);
                for (male_idx = 0; male_idx < male_alternates->length; male_idx++) {
                    male_alternate = list_get(male_alternates, male_idx);
                    args.alternate = male_alternate;
                    if (!can_preload_room_alternate(&args))
                        continue;
                    list_append(child->male_alternates, male_alternate);
                    male_qm = preload_room_alternate(&args);
                    u = male_alternate->loc_u;
                    v = male_alternate->loc_v;
                    dx = calculate_room_dx(room, orientation, u, v);
                    dz = calculate_room_dz(room, orientation, u, v);
                    local_settings.male_x = origin_x + dx;
                    local_settings.male_z = origin_z + dz;
                    if (pregenerate_map_helper(global_settings, local_settings, child)) {
                        local_settings.num_rooms_loaded++;
                        if (roomset->branch(&game_api, &local_settings)) {
                            quadmask_destroy(male_qm);
                            male_qm = NULL;
                            male_idx = 0;
                            continue;
                        }
                        goto success;
                    } else {
                        unpreload_room_alternate(&args, male_qm);
                        quadmask_destroy(male_qm);
                        male_qm = NULL;
                        list_idx = list_search(child->male_alternates, male_alternate);
                        list_remove(child->male_alternates, list_idx);
                    }
                }
no_path:
                local_settings.no_path = true;
                if (pregenerate_map_helper(global_settings, local_settings, child))
                    goto success;
                list_destroy(male_alternates);
                map_node_detach(parent, child);
                map_node_destroy(child);
                local_settings.num_rooms_left++;
                args.alternate = female_alternate;
                unpreload_room_alternate(&args, fem_qm);
                quadmask_destroy(fem_qm);
                fem_qm = NULL;
                unpreload_room(&args);
            }
        }
        list_destroy(female_alternates);
    }
    list_destroy(rooms);
    return false;
success:
    list_destroy(male_alternates);
    list_destroy(female_alternates);
    list_destroy(rooms);
    if (fem_qm != NULL)
        quadmask_destroy(fem_qm);
    if (male_qm != NULL)
        quadmask_destroy(male_qm);
    return true;
}

typedef struct {
    Map* map;
    MapNode* node;
    Quadmask* qm;
    Palette* palette;
    Roomset* roomset;
    Room* room;
    Alternate* alternate;
    i32 origin_x;
    i32 origin_z;
    i32 orientation;
} LoadArgs;

static void place_tile(Map* map, TileColor* tile_color, i32 x, i32 z)
{
    Tile* tile = NULL;
    Wall* wall = NULL;
    vec2 position = vec2_create(x, z);
    if (tile_color->is_wall) {
        wall = wall_create(position, tile_color->height, tile_color->color);
        list_append(map->walls, wall);
        wall->side_tex = tile_color->side_tex;
        wall->top_tex = tile_color->top_tex;
        quadmask_set(map->tile_mask, x, z);
        map->tilemap[z * map->width + x] = wall;
    } else {
        tile = tile_create(position, tile_color->color);
        list_append(map->tiles, tile);
        tile->tex = tile_color->tex;
        tile->collide = tile_color->collide;
        if (tile_color->create != NULL)
            tile_color->create(&game_api, tile);
        map->tilemap[z * map->width + x] = tile;
    }
}

static void set_map_node(Map* map, MapNode* node, i32 map_x, i32 map_z)
{
    map->map_nodes[map_z * MAP_MAX_WIDTH + map_x] = node;
}

static void load_room(LoadArgs* args)
{
    Map* map = args->map;
    MapNode* node = args->node;
    Quadmask* qm = args->qm;
    Palette* palette = args->palette;
    Roomset* roomset = args->roomset;
    Room* room = args->room;
    i32 origin_x = args->origin_x;
    i32 origin_z = args->origin_z;
    i32 orientation = args->orientation;
    TileColor* tile_color;
    i32 u, v, map_x, map_z, dx, dz;
    u32 color;
    for (v = room->v1; v <= room->v2; v++) {
        for (u = room->u1; u <= room->u2; u++) {
            dx = calculate_room_dx(room, orientation, u, v);
            dz = calculate_room_dz(room, orientation, u, v);
            map_x = origin_x + dx;
            map_z = origin_z + dz;
            node->x1 = mini(node->x1, map_x);
            node->x2 = maxi(node->x2, map_x);
            node->z1 = mini(node->z1, map_z);
            node->z2 = maxi(node->z2, map_z);
            color = roomset_get_color(roomset, u, v);
            if (color_is_preset(color))
                continue;
            if (quadmask_isset(qm, map_x, map_z))
                continue;
            tile_color = palette_get(palette, color);
            if (tile_color == NULL)
                continue;
            quadmask_set(qm, map_x, map_z);
            set_map_node(map, node, map_x, map_z);
            place_tile(map, tile_color, map_x, map_z);
        }
    }
    if (room->create != NULL) {
        map_context.current_map_node = node;
        room->create(&game_api);
        map_context.current_map_node = NULL;
    }
}

static void load_room_alternate(LoadArgs* args)
{
    Map* map = args->map;
    MapNode* node = args->node;
    Quadmask* qm = args->qm;
    Palette* palette = args->palette;
    Roomset* roomset = args->roomset;
    Room* room = args->room;
    Alternate* alternate = args->alternate;
    i32 origin_x = args->origin_x;
    i32 origin_z = args->origin_z;
    i32 orientation = args->orientation;
    i32 u, v, map_x, map_z, dx, dz;
    TileColor* tile_color;
    u32 color;
    for (v = alternate->v1; v <= alternate->v2; v++) {
        for (u = alternate->u1; u <= alternate->u2; u++) {
            dx = calculate_alternate_dx(room, alternate, orientation, u, v);
            dz = calculate_alternate_dz(room, alternate, orientation, u, v);
            map_x = origin_x + dx;
            map_z = origin_z + dz;
            color = roomset_get_color(roomset, u, v);
            if (color_is_preset(color))
                continue;
            if (quadmask_isset(qm, map_x, map_z))
                continue;
            tile_color = palette_get(palette, color);
            if (tile_color == NULL)
                continue;
            quadmask_set(qm, map_x, map_z);
            set_map_node(map, node, map_x, map_z);
            place_tile(map, tile_color, map_x, map_z);
        }
    }
}

static void load_room_alternate_default(LoadArgs* args)
{
    Map* map = args->map;
    MapNode* node = args->node;
    Quadmask* qm = args->qm;
    Room* room = args->room;
    Alternate* alternate = args->alternate;
    i32 origin_x = args->origin_x;
    i32 origin_z = args->origin_z;
    i32 orientation = args->orientation;
    i32 u, v, dx, dz, map_x, map_z;
    TileColor* tile_color;
    
    u = alternate->loc_u;
    v = alternate->loc_v;
    dx = calculate_room_dx(room, orientation, u, v);
    dz = calculate_room_dz(room, orientation, u, v);
    map_x = origin_x + dx;
    map_z = origin_z + dz;
    if (quadmask_isset(qm, map_x, map_z))
        return;
    tile_color = alternate->default_tile;
    if (tile_color == NULL)
        return;
    quadmask_set(qm, map_x, map_z);
    set_map_node(map, node, map_x, map_z);
    place_tile(map, tile_color, map_x, map_z);
}

static void generate_map_helper(Map* map, Quadmask* qm, Palette* palette, Roomset* roomset, MapNode* node)
{
    Room* room = node->room;
    Alternate* alternate;
    LoadArgs args;
    i32 i;

    args = (LoadArgs) {
        .map = map,
        .node = node,
        .qm = qm,
        .palette = palette,
        .roomset = roomset,
        .room = node->room,
        .origin_x = node->origin_x,
        .origin_z = node->origin_z,
        .orientation = node->orientation
    };

    for (i = 0; i < node->male_alternates->length; i++) {
        args.alternate = list_get(node->male_alternates, i);
        load_room_alternate(&args);
    }

    if (room != NULL) {
        for (i = 0; i < room->male_alternates->length; i++) {
            alternate = list_get(room->male_alternates, i);
            args.alternate = alternate;
            if (!list_contains(node->male_alternates, alternate))
                load_room_alternate_default(&args);
        }
    }

    if (node->female_alternate != NULL) {
        args.alternate = node->female_alternate;
        load_room_alternate(&args);
    }

    if (room != NULL) {
        for (i = 0; i < room->female_alternates->length; i++) {
            alternate = list_get(room->female_alternates, i);
            args.alternate = alternate;
            if (alternate != node->female_alternate)
                load_room_alternate_default(&args);
        }
    }

    if (node->room != NULL) {
        args.room = node->room;
        load_room(&args);
    }

    for (i = 0; i < node->num_children; i++)
        generate_map_helper(map, qm, palette, roomset, node->children[i]);
}

static void fill_map(Map* map, Quadmask* qm, Palette* palette)
{
    i32 x, z;
    TileColor* tile_color;

    tile_color = palette_get(palette, 0);
    if (tile_color == NULL)
        return;

    for (x = 0; x < map->width; x++) {
        for (z = 0; z < map->length; z++) {
            if (!quadmask_isset(qm, x, z))
                place_tile(map, tile_color, x, z);
        }
    }
}

static Map* generate_map(i32 id)
{
    JsonValue* value;
    JsonObject* object;
    Palette* palette;
    Roomset* roomset;
    const char* path;
    Map* map;
    MapNode* root;
    GlobalMapGenerationSettings global_settings;
    LocalMapGenerationSettings local_settings;
    Quadmask* qm;

    value = json_object_get_value(map_context.json, map_context.names[id]);
    if (value == NULL)
        throw_map_error(ERROR_MISSING);
    if (json_value_get_type(value) != JTYPE_OBJECT)
        throw_map_error(ERROR_INVALID_TYPE);

    object = json_value_get_object(value);

    value = json_object_get_value(object, "path");
    if (value == NULL)
        throw_map_error(ERROR_MISSING);
    if (json_value_get_type(value) != JTYPE_STRING)
        throw_map_error(ERROR_INVALID_TYPE);

    map = st_malloc(sizeof(Map));
    qm = quadmask_create(MAP_MAX_WIDTH, MAP_MAX_LENGTH);
    path = json_value_get_string(value);
    palette = palette_create(object);
    roomset = roomset_create(object, path, palette);

    global_settings.qm = qm;
    global_settings.roomset = roomset;
    //global_settings.num_branches = 0;
    local_settings.current_branch = "main";
    local_settings.current_room_type = "spawn";
    local_settings.num_rooms_left = 1;
    local_settings.male_x = MAP_MAX_WIDTH / 2;
    local_settings.male_z = MAP_MAX_LENGTH / 2;
    local_settings.no_path = false;
    local_settings.create_no_path = false;
    local_settings.succeed_even_if_no_path = false;

    root = map_node_create();

    map->width = MAP_MAX_WIDTH;
    map->length = MAP_MAX_LENGTH;
    map->root = NULL;
    map->roomset = roomset;
    map->tile_mask = quadmask_create(MAP_MAX_WIDTH, MAP_MAX_LENGTH);
    map->fog_mask = quadmask_create(MAP_MAX_WIDTH, MAP_MAX_LENGTH);
    map->tilemap = st_calloc(map->width * map->length, sizeof(void*));
    map->map_nodes = st_calloc(map->width * map->length, sizeof(MapNode*));
    map->bosses = list_create();
    map->entities = list_create();
    map->tiles = list_create();
    map->walls = list_create();
    map->free_walls = list_create();
    map->projectiles = list_create();
    map->obstacles = list_create();
    map->parstacles = list_create();
    map->particles = list_create();
    map->parjicles = list_create();
    map->triggers = list_create();
    map->aoes = list_create();
    map->root = root;
    map->spawn_point = vec2_create(MAP_MAX_WIDTH / 2 + 0.5, MAP_MAX_LENGTH / 2 + 0.5);
    map->active = true;

    map_context.current_map = map;

    if (!pregenerate_map_helper(&global_settings, local_settings, root))
        throw_map_error(ERROR_GENERIC);

    quadmask_clear(qm);

    generate_map_helper(map, qm, palette, roomset, root);
    fill_map(map, qm, palette);

    map_context.current_map = NULL;

    palette_destroy(palette);
    quadmask_destroy(qm);

    return map;
}

static MapNode* get_map_node(Map* map, i32 x, i32 z)
{
    log_assert(map != NULL, "map is null");
    if (x < 0 || z < 0 || x >= MAP_MAX_WIDTH || z >= MAP_MAX_WIDTH)
        return NULL;
    return map->map_nodes[z * MAP_MAX_WIDTH + x];
}

static void clear_map_node_fog(Map* map, MapNode* node)
{
    log_assert(map != NULL, "map is null");
    if (node->cleared)
        return;
    node->cleared = true;
    log_write(DEBUG, "%d %d %d %d", node->z1, node->z2, node->x1, node->x2);
    for (i32 z = node->z1; z <= node->z2; z++)
        for (i32 x = node->x1; x <= node->x2; x++)
            if (node == get_map_node(map, x, z))
                quadmask_set(map->fog_mask, x, z);
}

bool map_fog_contains(Map* map, vec2 position)
{
    log_assert(map != NULL, "map is null");
    i32 x, z;
    x = (i32) position.x;
    z = (i32) position.z;
    if (map == NULL)
        return false;
    if (!quadmask_in_bounds(map->fog_mask, x, z))
        return false;
    return !quadmask_isset(map->fog_mask, x, z);
}

bool map_fog_contains_tile(Map* map, Tile* tile)
{
    log_assert(map != NULL, "map is null");
    return map_fog_contains(map, tile->position);

    //Map* map = map_context.current_map;
    //i32 x, z;
    //x = (i32)roundf(tile->position.x);
    //z = (i32)roundf(tile->position.z);
    //if (map == NULL)
    //    return false;
    //if (tile != map_get_tile(x, z))
    //    return false;
    //if (!quadmask_in_bounds(map->fog_mask, x, z))
    //    return false;
    //return !quadmask_isset(map->fog_mask, x, z);
}

bool map_fog_contains_wall(Map* map, Wall* wall)
{
    log_assert(map != NULL, "map is null");
    return map_fog_contains(map, wall->position);
}

static void current_map_node_exit(Map* map, MapNode* node)
{
    log_assert(map != NULL, "map is null");
    if (node == NULL)
        return;
    if (node->room->exit == NULL)
        return;
    node->room->exit(&game_api, node->num_exits++);
}

static void current_map_node_enter(Map* map, MapNode* node)
{
    log_assert(map != NULL, "map is null");
    log_assert(node != NULL, "node is null");
    if (node == NULL)
        return;
    if (node->room->enter == NULL)
        return;
    node->room->enter(&game_api, node->num_enters++);
}

void map_fog_explore(Map* map, vec2 position)
{
    log_assert(map != NULL, "map is null");
    i32 x, z;
    x = (i32) position.x;
    z = (i32) position.z;
    if (map == NULL)
        return;
    MapNode* node = get_map_node(map, x, z);
    if (node == NULL)
        return;
    //if (map_context.current_map_node != node) {
    //    current_map_node_exit(map_context.current_map, map_context.current_map_node);
    //    map_context.current_map_node = node;
    //    current_map_node_enter(map_context.current_map, map_context.current_map_node);
    //}
    if (node->visited) 
        return;
    if (!quadmask_in_bounds(map->fog_mask, x, z))
        return;
    node->visited = true;
    clear_map_node_fog(map, node);
    // this should never occur
    if (node->parent != NULL)
        clear_map_node_fog(map, node->parent);
    for (i32 i = 0; i < node->num_children; i++)
        clear_map_node_fog(map, node->children[i]);
    game_render_update_walls();
    game_render_update_tiles();
    game_render_update_obstacles();
    game_render_update_parstacles();
}

void map_fog_clear(Map* map)
{
    log_assert(map != NULL, "map is null");
    if (map == NULL)
        return;
    quadmask_setall(map->fog_mask);
    game_render_update_walls();
    game_render_update_tiles();
    game_render_update_obstacles();
    game_render_update_parstacles();
}

void map_handle_trigger_enter(Trigger* trigger, Entity* entity)
{
    map_context.current_map_node = trigger->map_node;
    if (trigger->enter != NULL)
        trigger->enter(&game_api, trigger, entity);
    map_context.current_map_node = NULL;
}

void map_handle_trigger_stay(Trigger* trigger, Entity* entity)
{
    map_context.current_map_node = trigger->map_node;
    if (trigger->stay != NULL)
        trigger->stay(&game_api, trigger, entity);
    map_context.current_map_node = NULL;
}

void map_handle_trigger_leave(Trigger* trigger, Entity* entity)
{
    // in update loop so dont have to set map node
    if (trigger->leave != NULL)
        trigger->leave(&game_api, trigger, entity);
}

vec2 map_orientation(void)
{
    MapNode* node = map_context.current_map_node;
    vec2 vec;
    if (node == NULL)
        return vec2_create(0, 0);
    switch (node->orientation) {
        case ROTATION_R0:
            vec = vec2_create(0, 1);
            break;
        case ROTATION_R1:
            vec = vec2_create(1, 0);
            break;
        case ROTATION_R2:
            vec = vec2_create(0, -1);
            break;
        case ROTATION_R3:
            vec = vec2_create(-1, 0);
            break;
        case ROTATION_MR0:
            vec = vec2_create(0, 1);
            break;
        case ROTATION_MR1:
            vec = vec2_create(-1, 0);
            break;
        case ROTATION_MR2:
            vec = vec2_create(0, -1);
            break;
        case ROTATION_MR3:
            vec = vec2_create(1, 0);
            break;
        default:
            vec = vec2_create(0, 0);
            break;
    }
    return vec;
}

static void clear_map(void)
{
    Map* map = game_context.current_map;
    if (map == NULL)
        return;
    map_destroy(map);
    game_context.current_map = NULL;
    map_context.current_map_node = NULL;
}

void map_interactable_callback(InteractableFuncPtr fptr, Map* map, MapNode* map_node)
{
    map_context.current_map = map;
    map_context.current_map_node = map_node;
    fptr(&game_api);
}

void map_set_interactable(const char* desc, InteractableFuncPtr func_ptr)
{
    log_assert(map_context.current_map != NULL, "");
    if (!map_context.current_map->active)
        return;
    gui_set_interactable(desc, func_ptr, map_context.current_map, map_context.current_map_node);
}

void* map_get_data(void)
{
    log_assert(map_context.current_map != NULL, "");
    return map_context.current_map->roomset->data;
}

Parjicle* map_create_parjicle(vec3 position)
{
    Parjicle* parj;
    Map* map = map_context.current_map;
    if (!map->active)
        return NULL;
    parj = parjicle_create(position);
    list_append(map->parjicles, parj);
    return parj;
}

Particle* map_create_particle(vec3 position)
{
    Particle* part;
    Map* map = map_context.current_map;
    if (!map->active)
        return NULL;
    part = particle_create(position);
    list_append(map->particles, part);
    return part;
}

Projectile* map_create_projectile(vec2 position)
{
    Projectile* proj;
    Map* map = map_context.current_map;
    if (!map->active)
        return NULL;
    proj = projectile_create(position);
    list_append(map->projectiles, proj);
    return proj;
}

Trigger* map_create_trigger(vec2 position, f32 radius)
{
    Trigger* trigger;
    Map* map = map_context.current_map;
    MapNode* node = map_context.current_map_node;
    if (!map->active)
        return NULL;
    trigger = trigger_create(position, radius);
    trigger->map_node = node;
    list_append(map->triggers, trigger);
    return trigger;
}

AOE* map_create_aoe(vec2 position, f32 lifetime)
{
    AOE* aoe;
    Map* map = map_context.current_map;
    if (!map->active)
        return NULL;
    aoe = aoe_create(position, lifetime);
    list_append(map->aoes, aoe);
    return aoe;
}

Entity* room_create_entity_explicit(Map* map, MapNode* node, vec2 position, i32 id)
{
    if (!map->active)
        return NULL;
    log_assert(node != NULL, "fuck");
    Room* room = node->room;
    i32 orientation = node->orientation;
    f32 u = room->u1 + position.x;
    f32 v = room->v1 + position.z;
    f32 dx = calculate_room_dxf(room, orientation, u, v);
    f32 dz = calculate_room_dzf(room, orientation, u, v);
    vec2 new_position;
    new_position.x = node->origin_x + dx;
    new_position.z = node->origin_z + dz;
    Entity* entity = entity_create(new_position, id);
    entity->map_info.spawn_node = node;
    entity->map_info.current_node = NULL;
    list_append(map->entities, entity);
    return entity;
}

Entity* room_create_entity(vec2 position, i32 id)
{
    Map* map = map_context.current_map;
    MapNode* node = map_context.current_map_node;
    return room_create_entity_explicit(map, node, position, id);
}

Trigger* room_create_trigger(vec2 position, f32 radius)
{
    Trigger* trigger;
    Map* map = map_context.current_map;
    MapNode* node = map_context.current_map_node;
    if (!map->active)
        return NULL;
    log_assert(node != NULL, "fuck");
    Room* room = node->room;
    i32 orientation = node->orientation;
    f32 u = room->u1 + position.x;
    f32 v = room->v1 + position.z;
    f32 dx = calculate_room_dxf(room, orientation, u, v);
    f32 dz = calculate_room_dzf(room, orientation, u, v);
    vec2 new_position;
    new_position.x = node->origin_x + dx;
    new_position.z = node->origin_z + dz;
    trigger = trigger_create(new_position, radius);
    trigger->map_node = node;
    list_append(map->triggers, trigger);
    return trigger;
}

Obstacle* room_create_obstacle(vec2 position)
{
    Obstacle* obstacle;
    Map* map = map_context.current_map;
    MapNode* node = map_context.current_map_node;
    if (!map->active)
        return NULL;
    log_assert(node != NULL, "fuck");
    Room* room = node->room;
    i32 orientation = node->orientation;
    f32 u = room->u1 + position.x;
    f32 v = room->v1 + position.z;
    f32 dx = calculate_room_dxf(room, orientation, u, v);
    f32 dz = calculate_room_dzf(room, orientation, u, v);
    vec2 new_position;
    new_position.x = node->origin_x + dx;
    new_position.z = node->origin_z + dz;
    obstacle = obstacle_create(new_position);
    list_append(map->obstacles, obstacle);
    return obstacle;
}

Parstacle* room_create_parstacle(vec2 position)
{
    Parstacle* parstacle;
    Map* map = map_context.current_map;
    MapNode* node = map_context.current_map_node;
    if (!map->active)
        return NULL;
    log_assert(node != NULL, "fuck");
    Room* room = node->room;
    i32 orientation = node->orientation;
    f32 u = room->u1 + position.x;
    f32 v = room->v1 + position.z;
    f32 dx = calculate_room_dxf(room, orientation, u, v);
    f32 dz = calculate_room_dzf(room, orientation, u, v);
    vec2 new_position;
    new_position.x = node->origin_x + dx;
    new_position.z = node->origin_z + dz;
    parstacle = parstacle_create(new_position);
    list_append(map->parstacles, parstacle);
    return parstacle;
}

Wall* room_create_wall(vec2 position, f32 height, f32 width, f32 length, u32 minimap_color)
{
    Map* map = map_context.current_map;
    MapNode* node = map_context.current_map_node;
    if (!map->active)
        return NULL;
    log_assert(node != NULL, "fuck");

    Room* room = node->room;
    i32 orientation = node->orientation;
    f32 u, v, dx1, dz1, dx2, dz2;
    vec2 new_position;
    u = room->u1 + position.x;
    v = room->v1 + position.z;
    dx1 = calculate_room_dxf(room, orientation, u, v);
    dz1 = calculate_room_dzf(room, orientation, u, v);
    dx2 = calculate_room_dxf(room, orientation, u+width, v+length);
    dz2 = calculate_room_dzf(room, orientation, u+width, v+length);
    new_position.x = node->origin_x + minf(dx1, dx2);
    new_position.z = node->origin_z + minf(dz1, dz2);
    Wall* wall = wall_create(new_position, height, minimap_color);
    list_append(map->walls, wall);
    list_append(map->free_walls, wall);
    game_render_update_walls();
    i32 mod = orientation % 2;
    wall->size.x = width * (1-mod) + length * (mod); 
    wall->size.y = width * (mod) + length * (1-mod); 
    return wall;
}

Tile* room_set_tilemap_tile(i32 x, i32 z, u32 minimap_color)
{
    Tile* tile;
    Map* map = map_context.current_map;
    MapNode* node = map_context.current_map_node;
    if (!map->active)
        return NULL;
    log_assert(node != NULL, "fuck");
    Room* room = node->room;
    i32 orientation = node->orientation;
    i32 u, v, dx, dz;
    u = room->u1 + x;
    v = room->v1 + z;
    dx = calculate_room_dx(room, orientation, u, v);
    dz = calculate_room_dz(room, orientation, u, v);
    x = node->origin_x + dx;
    z = node->origin_z + dz;
    tile = tile_create(vec2_create(x, z), minimap_color);
    list_append(map->tiles, tile);
    map_set_tile(map_context.current_map, x, z, tile);
    return tile;
}

Wall* room_set_tilemap_wall(i32 x, i32 z, f32 height, u32 minimap_color)
{
    Wall* wall;
    Map* map = map_context.current_map;
    MapNode* node = map_context.current_map_node;
    if (!map->active)
        return NULL;
    log_assert(node != NULL, "fuck");
    Room* room = node->room;
    i32 orientation = node->orientation;
    i32 u, v, dx, dz;
    u = room->u1 + x;
    v = room->v1 + z;
    dx = calculate_room_dx(room, orientation, u, v);
    dz = calculate_room_dz(room, orientation, u, v);
    x = node->origin_x + dx;
    z = node->origin_z + dz;
    wall = wall_create(vec2_create(x, z), height, minimap_color);
    list_append(map->walls, wall);
    map_set_wall(map, x, z, wall);
    return wall;
}

vec2 room_position(vec2 position)
{
    log_write(FATAL, "Unused function for now");
    return vec2_create(0,0);
    //Map* map = map_context.current_map;
    //MapNode* node = map_context.current_map_node;
    //if (!map->active) {
    //    log_write(WARNING, "Map is not active");
    //    return vec2_create(0,0);
    //}
    //log_assert(node != NULL, "fuck");
    //Room* room = node->room;
    //i32 orientation = node->orientation;
    //i32 u, v, dx, dz;
    //log_write(DEBUG, "B: %f %f", position.x, position.z);
    //dx = position.x - node->origin_x;
    //dz = position.z - node->origin_z;
    //u = room->u1 + position.x - node->origin_x;
    //v = room->v1 + position.z - node->origin_z;
    //dx = calculate_room_dx(room, orientation, u, v);
    //dz = calculate_room_dz(room, orientation, u, v);
    //position.x = node->origin_x + dx;
    //position.z = node->origin_z + dz;
    //log_write(DEBUG, "C: %f %f", position.x, position.z);
    //return position;
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

bool map_is_wall(Map* map, i32 x, i32 z)
{
    log_assert(map != NULL, "map is null");
    if (map == NULL) return false;
    if (x < 0 || x >= map->width) return false;
    if (z < 0 || z >= map->length) return false;
    return quadmask_isset(map->tile_mask, x, z);
}

void* map_get(Map* map, i32 x, i32 z)
{
    log_assert(map != NULL, "map is null");
    if (map == NULL) return NULL;
    if (x < 0 || x >= map->width) return NULL;
    if (z < 0 || z >= map->length) return NULL;
    return map->tilemap[z * map->width + x];
}

Tile* map_get_tile(Map* map, i32 x, i32 z)
{
    log_assert(map != NULL, "map is null");
    if (map == NULL) return NULL;
    if (x < 0 || x >= map->width) return NULL;
    if (z < 0 || z >= map->length) return NULL;
    if (quadmask_isset(map->tile_mask, x, z))
        return NULL;
    return map->tilemap[z * map->width + x];
}

Wall* map_get_wall(Map* map, i32 x, i32 z)
{
    log_assert(map != NULL, "map is null");
    if (map == NULL) return NULL;
    if (x < 0 || x >= map->width) return NULL;
    if (z < 0 || z >= map->length) return NULL;
    if (!quadmask_isset(map->tile_mask, x, z))
        return NULL;
    return map->tilemap[z * map->width + x];
}

void map_set_tile(Map* map, i32 x, i32 z, Tile* tile)
{
    log_assert(map != NULL, "map is null");
    void* prev_tile;
    if (map == NULL) return;
    if (x < 0 || x >= map->width) return;
    if (z < 0 || z >= map->length) return;
    prev_tile = map->tilemap[z * map->width + x];
    if (quadmask_isset(map->tile_mask, x, z))
        wall_set_flag(prev_tile, WALL_FLAG_ACTIVE, false);
    else
        tile_set_flag(prev_tile, TILE_FLAG_ACTIVE, false);
    map->tilemap[z * map->width + x] = tile;
    quadmask_unset(map->tile_mask, x, z);
    game_render_update_tiles();
    game_render_update_walls();
}

void map_set_wall(Map* map, i32 x, i32 z, Wall* wall)
{
    log_assert(map != NULL, "map is null");
    void* prev_tile;
    if (map == NULL) return;
    if (x < 0 || x >= map->width) return;
    if (z < 0 || z >= map->length) return;
    prev_tile = map->tilemap[z * map->width + x];
    if (quadmask_isset(map->tile_mask, x, z))
        wall_set_flag(prev_tile, WALL_FLAG_ACTIVE, false);
    else
        tile_set_flag(prev_tile, TILE_FLAG_ACTIVE, false);
    map->tilemap[z * map->width + x] = wall;
    quadmask_set(map->tile_mask, x, z);
    game_render_update_tiles();
    game_render_update_walls();
}

void map_init(void)
{
    JsonObject* json = state_context.config->maps;
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

        name = json_member_get_key(member);
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

void map_cleanup(void)
{
    st_free(map_context.names);
    map_destroy(game_context.current_map);
}

Map* map_create(i32 id)
{
    Map* map;
    Entity* entity;

    if (id == -1) {
        log_write(WARNING, "Tried to load map with id of -1");
        return NULL;
    }

    game_render_update_obstacles();
    game_render_update_parstacles();
    game_render_update_tiles();
    game_render_update_walls();

    clear_map();
    game_context.player.entity = NULL;

    map = generate_map(id);
    entity = entity_create(map->spawn_point, 0); 
    player_reset(entity);
    list_append(map->entities, entity);

    game_context.current_map = map;

    log_write(DEBUG, "loaded");

    return map;
}

static void destroy_entities(Map* map)
{
    Entity* entity;
    i32 i;
    for (i = 0; i < map->entities->length; i++) {
        entity =  list_get(map->entities, i);
        if (entity_get_flag(entity, ENTITY_FLAG_BOSS))
            gui_destroy_boss_healthbar(entity);
        map_context.current_map_node = entity->map_info.spawn_node;
        entity_destroy(entity);
        map_context.current_map_node = NULL;
    }
    list_destroy(map->entities);
}

static void destroy_projectiles(Map* map)
{
    Projectile* proj;
    i32 i;
    for (i = 0; i < map->projectiles->length; i++) {
        proj = list_get(map->projectiles, i);
        projectile_destroy(proj);
    }
    list_destroy(map->projectiles);
}

static void destroy_obstacles(Map* map)
{
    Obstacle* obstacle;
    i32 i;
    for (i = 0; i < map->obstacles->length; i++) {
        obstacle = list_get(map->obstacles, i);
        obstacle_destroy(obstacle);
    }
    list_destroy(map->obstacles);
}

static void destroy_parstacles(Map* map)
{
    Parstacle* parstacle;
    i32 i;
    for (i = 0; i < map->parstacles->length; i++) {
        parstacle = list_get(map->parstacles, i);
        parstacle_destroy(parstacle);
    }
    list_destroy(map->parstacles);
}

static void destroy_particles(Map* map)
{
    Particle* particle;
    i32 i;
    for (i = 0; i < map->particles->length; i++) {
        particle = list_get(map->particles, i);
        particle_destroy(particle);
    }
    list_destroy(map->particles);
}

static void destroy_parjicles(Map* map)
{
    Parjicle* parjicle;
    i32 i;
    for (i = 0; i < map->parjicles->length; i++) {
        parjicle = list_get(map->parjicles, i);
        parjicle_destroy(parjicle);
    }
    list_destroy(map->parjicles);
}

static void destroy_triggers(Map* map)
{
    Trigger* trigger;
    i32 i;
    for (i = 0; i < map->triggers->length; i++) {
        trigger = list_get(map->triggers, i);
        trigger_destroy(trigger);
    }
    list_destroy(map->triggers);
}

static void destroy_aoes(Map* map)
{
    AOE* aoe;
    i32 i;
    for (i = 0; i < map->aoes->length; i++) {
        aoe = list_get(map->aoes, i);
        aoe_destroy(aoe);
    }
    list_destroy(map->aoes);
}

void map_destroy(Map* map)
{
    i32 i;
    map_context.current_map = map;
    map->active = false;
    destroy_entities(map);
    destroy_projectiles(map);
    destroy_obstacles(map);
    destroy_parstacles(map);
    destroy_particles(map);
    destroy_parjicles(map);
    destroy_triggers(map);
    destroy_aoes(map);
    for (i = 0; i < map->tiles->length; i++)
        tile_destroy(list_get(map->tiles, i));
    list_destroy(map->tiles);
    for (i = 0; i < map->walls->length; i++)
        wall_destroy(list_get(map->walls, i));
    list_destroy(map->walls);
    list_destroy(map->free_walls);
    list_destroy(map->bosses);
    st_free(map->tilemap);
    st_free(map->map_nodes);
    quadmask_destroy(map->tile_mask);
    quadmask_destroy(map->fog_mask);
    map_node_destroy(map->root);
    roomset_destroy(map->roomset);
    st_free(map);
}

static void map_collide_tilemap(Map* map)
{
    vec2 pos;
    f32 r;
    i32 i, x, z;
    Tile* tile;
    Wall* wall;
    for (i = 0; i < map->entities->length; i++) {
        Entity* entity = list_get(map->entities, i);
        pos = entity->position;
        r = entity->size / 2;
        for (x = floor(pos.x-r); x <= ceil(pos.x+r); x++) {
            for (z = floor(pos.z-r); z <= ceil(pos.z+r); z++) {
                tile = map_get_tile(map, x, z);
                wall = map_get_wall(map, x, z);
                if (tile != NULL)
                    collide_entity_tile(entity, tile);
                if (wall != NULL)
                    collide_entity_wall(entity, wall);
            }
        }
    }
    for (i = 0; i < map->projectiles->length; i++) {
        Projectile* projectile = list_get(map->projectiles, i);
        pos = projectile->position;
        r = projectile->size / 2;
        for (x = floor(pos.x-r); x <= ceil(pos.x+r); x++) {
            for (z = floor(pos.z-r); z <= ceil(pos.z+r); z++) {
                wall = map_get_wall(map, x, z);
                if (wall != NULL)
                    collide_projectile_wall(projectile, wall);
            }
        }
    }
}

void map_collide_objects(Map* map)
{
    i32 i, j;
    for (i = 0; i < map->entities->length; i++) {
        Entity* entity = list_get(map->entities, i);
        for (j = 0; j < map->obstacles->length; j++) {
            Obstacle* obstacle = list_get(map->obstacles, j);
            collide_entity_obstacle(entity, obstacle);
        }
        for (j = 0; j < map->free_walls->length; j++) {
            Wall* wall = list_get(map->free_walls, j);
            collide_entity_wall(entity, wall);
        }
        for (j = 0; j < map->projectiles->length; j++) {
            Projectile* projectile = list_get(map->projectiles, j);
            collide_entity_projectile(entity, projectile);
        }
        for (j = 0; j < map->triggers->length; j++) {
            Trigger* trigger = list_get(map->triggers, j);
            collide_entity_trigger(entity, trigger);
        }
        for (j = 0; j < map->aoes->length; j++) {
            AOE* aoe = list_get(map->aoes, j);
            if  (aoe->timer >= 0) continue;
            collide_entity_aoe(entity, aoe);
        }
    }
    for (i = 0; i < map->projectiles->length; i++) {
        Projectile* projectile = list_get(map->projectiles, i);
        if (projectile->lifetime <= 0) continue;
        for (j = 0; j < map->obstacles->length; j++) {
            Obstacle* obstacle = list_get(map->obstacles, j);
            collide_projectile_obstacle(projectile, obstacle);
        }
        for (j = 0; j < map->free_walls->length; j++) {
            Wall* wall = list_get(map->free_walls, j);
            collide_projectile_wall(projectile, wall);
        }
    }
}

static void map_update_objects(Map* map)
{
    i32 i, once, used, delete;
    // trigger updates MUST be before entity updates since trigger updates
    // will call functions on entities
    i = 0;
    while (i < map->triggers->length) {
        Trigger* trigger = list_get(map->triggers, i);
        map_context.current_map_node = trigger->map_node;
        delete = trigger_get_flag(trigger, TRIGGER_FLAG_DELETE);
        once = trigger_get_flag(trigger, TRIGGER_FLAG_ONCE);
        used = trigger_get_flag(trigger, TRIGGER_FLAG_USED);
        if ((once && used) || delete)
            trigger_destroy(list_remove(map->triggers, i));
        else {
            trigger_update(trigger);
            i++;
        }
    }
    map_context.current_map_node = NULL;
    i = 0;
    while (i < map->entities->length) {
        Entity* entity = list_get(map->entities, i);
        map_context.current_map_node = entity->map_info.spawn_node;
        entity_update(entity, game_context.dt);
        if (entity->health <= 0) {
            if (entity_get_flag(entity, ENTITY_FLAG_BOSS))
                map_unmake_boss(entity);
            entity_destroy(list_remove(map->entities, i));
        } else
            i++;
    }
    map_context.current_map_node = NULL;
    i = 0;
    while (i < map->projectiles->length) {
        Projectile* projectile = list_get(map->projectiles, i);
        projectile_update(projectile, game_context.dt);
        if (projectile->lifetime <= 0)
            projectile_destroy(list_remove(map->projectiles, i));
        else
            i++;
    }
    i = 0;
    while (i < map->particles->length) {
        Particle* particle = list_get(map->particles, i);
        particle_update(particle, game_context.dt);
        if (particle->lifetime <= 0)
            particle_destroy(list_remove(map->particles, i));
        else
            i++;
    }
    i = 0;
    while (i < map->parjicles->length) {
        Parjicle* parjicle = list_get(map->parjicles, i);
        parjicle_update(parjicle, game_context.dt);
        if (parjicle->lifetime <= 0)
            parjicle_destroy(list_remove(map->parjicles, i));
        else
            i++;
    }
    i = 0;
    while (i < map->aoes->length) {
        AOE* aoe = list_get(map->aoes, i);
        aoe_update(aoe, game_context.dt);
        if ((!aoe_get_flag(aoe, AOE_FLAG_LINGER) && aoe_get_flag(aoe, AOE_FLAG_USED)) || (aoe_get_flag(aoe, AOE_FLAG_LINGER) && aoe->lifetime <= 0))
            aoe_destroy(list_remove(map->aoes, i));
        else {
            aoe_set_flag(aoe, AOE_FLAG_USED, true);
            i++;
        }
    }
    player_update(&game_context.player, game_context.dt);
}

void map_update(Map* map)
{
    if (map == NULL)
        return;
    map_context.current_map = map;
    map_update_objects(map);
    map_collide_tilemap(map);
    map_collide_objects(map);
    camera_update();
    map_context.current_map = NULL;
}

void map_set_active(Map* map)
{
    map->active = true;
}

void map_set_inactive(Map* map)
{
    map->active = false;
}

void gui_create_boss_healthbar(char* name, Entity* boss);
void map_make_boss(char* name, Entity* entity)
{
    Map* map = map_context.current_map;
    if (!map->active)
        return;
    list_append(map->bosses, entity);
    log_assert(!entity_get_flag(entity, ENTITY_FLAG_BOSS), "Entity is already boss");
    entity_set_flag(entity, ENTITY_FLAG_BOSS, 1);
    //event_create_gui_create_boss_healthbar(name, entity, entity->health, entity->max_health);
    gui_create_boss_healthbar(name, entity);
    pthread_mutex_lock(&game_context.getter_mutex);
    pthread_mutex_unlock(&game_context.getter_mutex);
}

void map_unmake_boss(Entity* entity)
{
    Map* map = map_context.current_map;
    if (!map->active)
        return;
    log_assert(entity_get_flag(entity, ENTITY_FLAG_BOSS), "Entity is already not boss");
    for (i32 i = 0; i < map->bosses->length; i++) {
        if (list_get(map->bosses, i) == entity) {
            list_remove(map->bosses, i);
            goto found;
        }
    }
    log_write(FATAL, "boss not in bosses list");
found:
    entity_set_flag(entity, ENTITY_FLAG_BOSS, 0);
    gui_destroy_boss_healthbar(entity);
    pthread_mutex_lock(&game_context.getter_mutex);
    pthread_mutex_unlock(&game_context.getter_mutex);
}


List* map_list_entities(Map* map)
{
    return map->entities;
}

List* map_list_tiles(Map* map)
{
    return map->tiles;
}

List* map_list_walls(Map* map)
{
    return map->walls;
}

List* map_list_projectiles(Map* map)
{
    return map->projectiles;
}

List* map_list_obstacles(Map* map)
{
    return map->obstacles;
}

List* map_list_parstacles(Map* map)
{
    return map->parstacles;
}

List* map_list_particles(Map* map)
{
    return map->particles;
}

List* map_list_parjicles(Map* map)
{
    return map->parjicles;
}

List* map_list_triggers(Map* map)
{
    return map->triggers;
}
