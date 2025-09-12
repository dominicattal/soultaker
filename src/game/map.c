#include "internal.h"
#include "../state.h"
#include "../api.h"
#include <json.h>
#include <math.h>
#include <windows.h>
#include <stb_image.h>

#define DEFAULT_WALL_HEIGHT 1.5f
#define MAP_MAX_WIDTH   512
#define MAP_MAX_LENGTH  512
#define WHITE   0xFFFFFF
#define GRAY    0x808080
#define BLACK   0x000000

typedef void (*RoomCreateFuncPtr)(GameApi*);
typedef void (*TileCreateFuncPtr)(GameApi*, Tile*);
typedef void* (*RoomsetInitFuncPtr)(GameApi*);
// true if at end of branch, false otherwise
typedef bool (*RoomsetGenerateFuncPtr)(GameApi*, LocalMapGenerationSettings*);
// true if should create branch, false otherwise
typedef bool (*RoomsetBranchFuncPtr)(GameApi*, void*, LocalMapGenerationSettings*);
typedef void (*RoomsetCleanupFuncPtr)(GameApi*, void*);

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

typedef struct {
    i32 u1, v1, u2, v2;
    const char* name;
    const char* type;
    RoomCreateFuncPtr create;
    List* male_alternates;
    List* female_alternates;
} Room;

typedef struct {
    i32 width, length;
    u32* data;
    Room* rooms;
    i32 num_rooms;
    Palette* palette;
    RoomsetInitFuncPtr init;
    RoomsetGenerateFuncPtr generate;
    RoomsetBranchFuncPtr branch;
    RoomsetCleanupFuncPtr cleanup;
} Roomset;

// Think of the map like a tree with rooms as nodes.
// Spawn room is like the root of the tree, and there are
// branches the player may go down that may lead to an event
// or a boss room.

typedef struct MapNode MapNode;

typedef struct {
    Quadmask* qm;
    Roomset* roomset;
    void* data;
} GlobalMapGenerationSettings;

typedef struct MapNode {
    MapNode* parent;
    MapNode** children;
    i32 num_children;
    Room* room;
    Alternate* female_alternate;
    List* male_alternates;
    i32 origin_x, origin_z;
    i32 x1, x2, z1, z2;
    i32 orientation;
    bool visited;
    bool cleared;
} MapNode;

typedef struct {
    i32 width, length;
    MapNode* root;
    void** tiles;
    MapNode** map_nodes;
    Quadmask* tile_mask;
    Quadmask* fog_mask;
} Map;

typedef struct {

    const char** names;
    i32 num_names;
    JsonObject* json;

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
    JsonValue* value = json_get_value(object, string);
    if (value == NULL)
        throw_map_error(ERROR_MISSING);
    if (json_get_type(value) != JTYPE_STRING)
        throw_map_error(ERROR_INVALID_TYPE);
    return json_get_string(value);
}

static JsonArray* get_array_value(JsonObject* object, const char* string)
{
    JsonValue* value = json_get_value(object, string);
    if (value == NULL)
        throw_map_error(ERROR_MISSING);
    if (json_get_type(value) != JTYPE_ARRAY)
        throw_map_error(ERROR_INVALID_TYPE);
    return json_get_array(value);
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
        color->name = json_member_key(member);
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
    if (json_get_type(value) != JTYPE_INT)
        throw_map_error(ERROR_INVALID_TYPE);
    return json_get_int(value);
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
    const char* string;
    string = get_string_value(object, "create");
    room->create = state_load_function(string);
    if (room->create == NULL)
        throw_map_error(ERROR_GENERIC);
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
    value = json_get_value(object, "alternates");
    if (value == NULL)
        return;
    if (json_get_type(value) != JTYPE_ARRAY)
        throw_map_error(ERROR_GENERIC);
    array = json_get_array(value);
    array_length = json_array_length(array);

    for (i = 0; i < array_length; i++) {
        alternate = st_malloc(sizeof(Alternate));
        value = json_array_get(array, i);
        if (json_get_type(value) != JTYPE_OBJECT)
            throw_map_error(ERROR_INVALID_TYPE);
        object = json_get_object(value);

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

        value = json_get_value(object, "default");
        if (value == NULL)
            alternate->default_tile = NULL;
        else {
            if (json_get_type(value) != JTYPE_STRING)
                throw_map_error(ERROR_INVALID_TYPE);
            string = json_get_string(value);
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
        u = room->u1 + alternate->loc_u;
        v = room->v1 + alternate->loc_v;
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
                if (alternate->loc_u != u-room->u1)
                    continue;
                if (alternate->loc_v != v-room->v1)
                    continue;
                goto found;
            }
            log_write(CRITICAL, "%s alternate not found for %d %d (%d %d)", string, u, v, u-room->u1, v-room->v1);
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
    u32* roomset_data;
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
    roomset_data = st_malloc(x * y * sizeof(u32));
    for (i = 0; i < y; i++) {
        for (j = 0; j < x; j++) {
            idx = i * x + j;
            roomset_data[idx] = raw_data[3*idx+2] + (raw_data[3*idx+1]<<8) + (raw_data[3*idx]<<16);
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

        parse_room_bounding_box(object, room);
        parse_room_create(object, room);
        parse_room_type(object, room);
        parse_room_alternates(object, palette, room);

        if (!verify_room_alternates(room, roomset_data, x, y))
            throw_map_error(ERROR_MISSING);

        json_iterator_increment(it);
    }
    json_iterator_destroy(it);

    roomset = st_malloc(sizeof(Roomset));
    roomset->rooms = rooms;
    roomset->num_rooms = num_rooms;
    roomset->data = roomset_data;
    roomset->width = x;
    roomset->length = y;
    roomset->palette = palette;
    roomset->generate = generate;
    roomset->branch = branch;
    roomset->init = init;
    roomset->cleanup = cleanup;

    return roomset;
}

static u32 roomset_get_color(Roomset* roomset, i32 u, i32 v)
{
    return roomset->data[v * roomset->width + u];
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
    st_free(roomset->data);
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

static f32 calculate_room_fdx(Room* room, Orientation orientation, f32 u, f32 v)
{
    f32 du, dv, w, l;
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

static f32 calculate_room_fdz(Room* room, Orientation orientation, f32 u, f32 v)
{
    f32 du, dv, w, l;
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
    ru = room->u1 + u - alternate->u1 + alternate->loc_u - alternate->origin_u;
    rv = room->v1 + v - alternate->v1 + alternate->loc_v - alternate->origin_v;
    return calculate_room_dx(room, orientation, ru, rv);
}

static i32 calculate_alternate_dz(Room* room, Alternate* alternate, i32 orientation, i32 u, i32 v)
{
    i32 ru, rv;
    ru = room->u1 + u - alternate->u1 + alternate->loc_u - alternate->origin_u;
    rv = room->v1 + v - alternate->v1 + alternate->loc_v - alternate->origin_v;
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
            room_color = roomset_get_color(roomset, room->u1+dx, room->v1+dz);
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

static void preload_room_alternate(PreloadArgs* args)
{
    Quadmask* qm = args->qm;
    Roomset* roomset = args->roomset;
    Room* room = args->room;
    Alternate* alternate = args->alternate;
    i32 origin_x = args->origin_x;
    i32 origin_z = args->origin_z;
    i32 orientation = args->orientation;
    u32 color;
    i32 u, v, dx, dz, map_x, map_z;
    for (v = alternate->v1; v <= alternate->v2; v++) {
        for (u = alternate->u1; u <= alternate->u2; u++) {
            dx = calculate_alternate_dx(room, alternate, orientation, u, v);
            dz = calculate_alternate_dz(room, alternate, orientation, u, v);
            color = roomset_get_color(roomset, u, v);
            map_x = origin_x + dx;
            map_z = origin_z + dz;
            quadmask_unset(qm, map_x, map_z);
            if (!color_is_preset(color))
                quadmask_set(qm, map_x, map_z);
        }
    }
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

static void unpreload_room_alternate(PreloadArgs* args)
{
    Quadmask* qm = args->qm;
    Roomset* roomset = args->roomset;
    Room* room = args->room;
    Alternate* alternate = args->alternate;
    i32 origin_x = args->origin_x;
    i32 origin_z = args->origin_z;
    i32 orientation = args->orientation;
    i32 u, v, ru, rv, dx, dz, map_x, map_z;
    u32 color;
    for (v = alternate->v1; v <= alternate->v2; v++) {
        for (u = alternate->u1; u <= alternate->u2; u++) {
            dx = calculate_alternate_dx(room, alternate, orientation, u, v);
            dz = calculate_alternate_dz(room, alternate, orientation, u, v);
            ru = room->u1 + dx;
            rv = room->v1 + dz;
            map_x = origin_x + dx;
            map_z = origin_z + dz;
            quadmask_unset(qm, map_x, map_z);
            color = roomset_get_color(roomset, ru, rv);
            if (color_is_preset(color))
                continue;
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
        return false;

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
                u = room->u1 + female_alternate->loc_u;
                v = room->v1 + female_alternate->loc_v;
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
                preload_room_alternate(&args);
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
                    preload_room_alternate(&args);
                    u = room->u1 + male_alternate->loc_u;
                    v = room->v1 + male_alternate->loc_v;
                    dx = calculate_room_dx(room, orientation, u, v);
                    dz = calculate_room_dz(room, orientation, u, v);
                    local_settings.male_x = origin_x + dx;
                    local_settings.male_z = origin_z + dz;
                    if (pregenerate_map_helper(global_settings, local_settings, child)) {
                        local_settings.num_rooms_loaded++;
                        if (roomset->branch(&game_api, global_settings->data, &local_settings))
                            continue;
                        break;
                    } else {
                        unpreload_room_alternate(&args);
                        list_idx = list_search(child->male_alternates, male_alternate);
                        list_remove(child->male_alternates, list_idx);
                    }
                }
                if (local_settings.num_rooms_loaded > 0)
                    goto success;
no_path:
                local_settings.no_path = true;
                if (pregenerate_map_helper(global_settings, local_settings, child))
                    goto success;
                list_destroy(male_alternates);
                map_node_detach(parent, child);
                map_node_destroy(child);
                local_settings.num_rooms_left++;
                args.alternate = female_alternate;
                unpreload_room_alternate(&args);
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
        wall = wall_create(position, tile_color->height);
        wall->side_tex = tile_color->side_tex;
        wall->top_tex = tile_color->top_tex;
        quadmask_set(map->tile_mask, x, z);
        map->tiles[z * map->width + x] = wall;
    } else {
        tile = tile_create(position);
        tile->tex = tile_color->tex;
        tile->collide = tile_color->collide;
        if (tile_color->create != NULL)
            tile_color->create(&game_api, tile);
        map->tiles[z * map->width + x] = tile;
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
            node->x1 = min(node->x1, map_x);
            node->x2 = max(node->x2, map_x);
            node->z1 = min(node->z1, map_z);
            node->z2 = max(node->z2, map_z);
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
    
    u = room->u1 + alternate->loc_u;
    v = room->v1 + alternate->loc_v;
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

static void generate_map(i32 id)
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

    qm = quadmask_create(MAP_MAX_WIDTH, MAP_MAX_LENGTH);
    path = json_get_string(value);
    palette = palette_create(object);
    roomset = roomset_create(object, path, palette);

    global_settings.qm = qm;
    global_settings.roomset = roomset;
    global_settings.data = roomset->init(&game_api);
    local_settings.current_branch = "main";
    local_settings.current_room_type = "spawn";
    local_settings.num_rooms_left = 1;
    local_settings.male_x = MAP_MAX_WIDTH / 2;
    local_settings.male_z = MAP_MAX_LENGTH / 2;
    local_settings.no_path = false;
    local_settings.create_no_path = false;

    root = map_node_create();

    if (!pregenerate_map_helper(&global_settings, local_settings, root))
        throw_map_error(ERROR_GENERIC);

    roomset->cleanup(&game_api, global_settings.data);

    map = st_malloc(sizeof(Map));
    map->width = MAP_MAX_WIDTH;
    map->length = MAP_MAX_LENGTH;
    map->root = root;
    map->tile_mask = quadmask_create(MAP_MAX_WIDTH, MAP_MAX_LENGTH);
    map->fog_mask = quadmask_create(MAP_MAX_WIDTH, MAP_MAX_LENGTH);
    map->tiles = st_calloc(map->width * map->length, sizeof(void*));
    map->map_nodes = st_calloc(map->width * map->length, sizeof(MapNode*));

    quadmask_clear(qm);
    generate_map_helper(map, qm, palette, roomset, root);

    map_context.current_map = map;

    game_set_player_position(vec2_create(MAP_MAX_WIDTH / 2 + 0.5, MAP_MAX_LENGTH / 2 + 0.5));

    palette_destroy(palette);
    roomset_destroy(roomset);
    quadmask_destroy(qm);
}

static MapNode* get_map_node(Map* map, i32 x, i32 z)
{
    if (x < 0 || z < 0 || x >= MAP_MAX_WIDTH || z >= MAP_MAX_WIDTH)
        return NULL;
    return map->map_nodes[z * MAP_MAX_WIDTH + x];
}

static void clear_map_node_fog(Map* map, MapNode* node)
{
    if (node->cleared)
        return;
    node->cleared = true;
    for (i32 z = node->z1; z < node->z2; z++)
        for (i32 x = node->x1; x < node->x2; x++)
            if (node == get_map_node(map, x, z))
                quadmask_set(map->fog_mask, x, z);
}

bool map_fog_contains(vec2 position)
{
    Map* map = map_context.current_map;
    i32 x, z;
    x = (i32) position.x;
    z = (i32) position.z;
    if (map == NULL)
        return false;
    if (!quadmask_in_bounds(map->fog_mask, x, z))
        return false;
    return !quadmask_isset(map->fog_mask, x, z);
}

bool map_fog_contains_tile(Tile* tile)
{
    return map_fog_contains(tile->position);

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

bool map_fog_contains_wall(Wall* wall)
{
    return map_fog_contains(wall->position);
}

void map_fog_explore(vec2 position)
{
    Map* map = map_context.current_map;
    i32 x, z;
    x = (i32) position.x;
    z = (i32) position.z;
    if (map == NULL)
        return;
    MapNode* node = get_map_node(map, x, z);
    if (node == NULL)
        return;
    if (node->visited) 
        return;
    if (!quadmask_in_bounds(map->fog_mask, x, z))
        return;
    node->visited = true;
    clear_map_node_fog(map, node);
    if (node->parent != NULL)
        clear_map_node_fog(map, node->parent);
    for (i32 i = 0; i < node->num_children; i++)
        clear_map_node_fog(map, node->children[i]);
    game_render_update_walls();
    game_render_update_tiles();
    game_render_update_obstacles();
    game_render_update_parstacles();
}

void map_fog_clear(void)
{
    Map* map = map_context.current_map;
    if (map == NULL)
        return;
    quadmask_setall(map->fog_mask);
    game_render_update_walls();
    game_render_update_tiles();
    game_render_update_obstacles();
    game_render_update_parstacles();
}

static void clear_map(void)
{
    Map* map = map_context.current_map;
    if (map == NULL)
        return;

    st_free(map->tiles);
    st_free(map->map_nodes);
    quadmask_destroy(map->tile_mask);
    quadmask_destroy(map->fog_mask);
    map_node_destroy(map->root);
    st_free(map);

    map_context.current_map = NULL;
}

Entity* room_create_entity(vec2 position, i32 id)
{
    MapNode* node = map_context.current_map_node;
    if (node == NULL)
        log_write(FATAL, "fuck");
    Room* room = node->room;
    i32 orientation = node->orientation;
    f32 u = room->u1 + position.x;
    f32 v = room->v1 + position.z;
    f32 dx = calculate_room_fdx(room, orientation, u, v);
    f32 dz = calculate_room_fdz(room, orientation, u, v);
    vec2 new_position;
    new_position.x = node->origin_x + dx;
    new_position.z = node->origin_z + dz;
    return entity_create(new_position, id);
}

Wall* room_create_wall(vec2 position, f32 height, f32 width, f32 length)
{
    MapNode* node = map_context.current_map_node;
    if (node == NULL)
        log_write(FATAL, "fuck");

    Room* room = node->room;
    i32 orientation = node->orientation;
    f32 u, v, dx1, dz1, dx2, dz2;
    vec2 new_position;
    u = room->u1 + position.x;
    v = room->v1 + position.z;
    dx1 = calculate_room_fdx(room, orientation, u, v);
    dz1 = calculate_room_fdz(room, orientation, u, v);
    dx2 = calculate_room_fdx(room, orientation, u+width, v+length);
    dz2 = calculate_room_fdz(room, orientation, u+width, v+length);
    new_position.x = node->origin_x + minf(dx1, dx2);
    new_position.z = node->origin_z + minf(dz1, dz2);
    Wall* wall = wall_create(new_position, height);
    i32 mod = orientation % 2;
    wall->size.x = width * (1-mod) + length * (mod); 
    wall->size.y = width * (mod) + length * (1-mod); 
    return wall;
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

bool map_is_wall(i32 x, i32 z)
{
    Map* map = map_context.current_map;
    if (map == NULL) return false;
    if (x < 0 || x >= map->width) return false;
    if (z < 0 || z >= map->length) return false;
    return quadmask_isset(map->tile_mask, x, z);
}

void* map_get(i32 x, i32 z)
{
    Map* map = map_context.current_map;
    if (map == NULL) return NULL;
    if (x < 0 || x >= map->width) return NULL;
    if (z < 0 || z >= map->length) return NULL;
    return map->tiles[z * map->width + x];
}

Tile* map_get_tile(i32 x, i32 z)
{
    Map* map = map_context.current_map;
    if (map == NULL) return NULL;
    if (x < 0 || x >= map->width) return NULL;
    if (z < 0 || z >= map->length) return NULL;
    if (quadmask_isset(map->tile_mask, x, z))
        return NULL;
    return map->tiles[z * map->width + x];
}

Wall* map_get_wall(i32 x, i32 z)
{
    Map* map = map_context.current_map;
    if (map == NULL) return NULL;
    if (x < 0 || x >= map->width) return NULL;
    if (z < 0 || z >= map->length) return NULL;
    if (!quadmask_isset(map->tile_mask, x, z))
        return NULL;
    return map->tiles[z * map->width + x];
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

    clear_map();
    generate_map(id);
    wall_update_free_walls();
    log_write(DEBUG, "loaded");
}

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

void map_cleanup(void)
{
    clear_map();
    st_free(map_context.names);
    st_free(map_context.current_map);
    json_object_destroy(map_context.json);
}
