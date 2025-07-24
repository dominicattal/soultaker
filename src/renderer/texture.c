#include "internal.h"
#include <assert.h>
#include <stdio.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include <stb_rect_pack.h>
#include <stb_truetype.h>
#include <math.h>
#include <string.h>
#include <json.h>

#define CHAR_OFFSET     32
#define NUM_CHARS       96
#define BITMAP_WIDTH    1024
#define BITMAP_HEIGHT   1024
#define PADDING         0

typedef struct {
    i16 font_size;
    i16 location;
    i32 ascent, descent, line_gap;
    struct {
        i32 advance, left_side_bearing;
        i32 kern[NUM_CHARS];
        i32 x1, y1, x2, y2;
        u16 u1, v1, u2, v2;
    } chars[NUM_CHARS];
} Font;

typedef struct {
    char* name;
    union { f32 u; i32 xint; };
    union { f32 v; i32 yint; };
    union { f32 w; i32 wint; };
    f32 h;
    vec2 pivot;
    vec2 stretch;
    i32 location;
} Texture;

typedef struct {
    Font fonts[NUM_FONTS];
    Texture* textures;
    struct {
        GLuint unit, name;
    } static_textures[NUM_TEXTURES];
    i32 num_textures;
    u32 texture_units[NUM_TEXTURE_UNITS];
} TextureContext;

static TextureContext texture_context;

static i32 texture_cmp(const void* ptr1, const void* ptr2)
{
    const Texture* tex1 = (const Texture*)ptr1;
    const Texture* tex2 = (const Texture*)ptr2;
    return strcmp(tex1->name, tex2->name);
}

static void load_font(stbtt_pack_context* spc, FontEnum font, i32 font_size, const char* ttf_path)
{
    unsigned char* font_buffer;
    stbtt_fontinfo info;
    stbtt_pack_range font_range;
    stbtt_packedchar chars[NUM_CHARS];
    i64 size;
    f32 scale;

    FILE* font_file = fopen(ttf_path, "rb");
    fseek(font_file, 0, SEEK_END);
    size = ftell(font_file);
    fseek(font_file, 0, SEEK_SET);
    font_buffer = st_malloc(size);
    fread(font_buffer, size, 1, font_file);
    fclose(font_file);
    
    stbtt_InitFont(&info, font_buffer, 0);

    font_range.font_size = font_size;         
    font_range.first_unicode_codepoint_in_range = CHAR_OFFSET; 
    font_range.array_of_unicode_codepoints = NULL;
    font_range.num_chars = NUM_CHARS;       
    font_range.chardata_for_range = chars;

    stbtt_PackFontRanges(spc, font_buffer, 0, &font_range, 1);

    scale = stbtt_ScaleForPixelHeight(&info, font_size);
    stbtt_GetFontVMetrics(&info, &texture_context.fonts[font].ascent, &texture_context.fonts[font].descent, &texture_context.fonts[font].line_gap);
    texture_context.fonts[font].ascent = roundf(texture_context.fonts[font].ascent * scale);
    texture_context.fonts[font].descent = roundf(texture_context.fonts[font].descent * scale);
    texture_context.fonts[font].line_gap = roundf(texture_context.fonts[font].line_gap * scale);
    texture_context.fonts[font].font_size = font_size;
    for (i32 i = 0; i < NUM_CHARS; i++) {
        texture_context.fonts[font].chars[i].u1 = chars[i].x0;
        texture_context.fonts[font].chars[i].v1 = chars[i].y0;
        texture_context.fonts[font].chars[i].u2 = chars[i].x1;
        texture_context.fonts[font].chars[i].v2 = chars[i].y1;
        stbtt_GetCodepointHMetrics(&info, i+CHAR_OFFSET, 
                                   &texture_context.fonts[font].chars[i].advance, 
                                   &texture_context.fonts[font].chars[i].left_side_bearing);
        texture_context.fonts[font].chars[i].advance = roundf(texture_context.fonts[font].chars[i].advance * scale);
        texture_context.fonts[font].chars[i].left_side_bearing = roundf(texture_context.fonts[font].chars[i].left_side_bearing * scale);
        stbtt_GetCodepointBitmapBox(&info, i+CHAR_OFFSET, scale, scale, 
                                    &texture_context.fonts[font].chars[i].x1,  
                                    &texture_context.fonts[font].chars[i].y1,  
                                    &texture_context.fonts[font].chars[i].x2,  
                                    &texture_context.fonts[font].chars[i].y2);
        for (i32 j = 0; j < NUM_CHARS; j++) {
            texture_context.fonts[font].chars[i].kern[j] = stbtt_GetCodepointKernAdvance(&info, i+CHAR_OFFSET, j+CHAR_OFFSET);
            texture_context.fonts[font].chars[i].kern[j] = roundf(texture_context.fonts[font].chars[i].kern[j] * scale);
        }
    }

    st_free(font_buffer);
}

void font_info(FontEnum font, i32 font_size, i32* ascent, i32* descent, i32* line_gap, i32* location)
{
    f32 scale = (f32)font_size / texture_context.fonts[font].font_size;
    *ascent   = roundf(texture_context.fonts[font].ascent   * scale);
    *descent  = roundf(texture_context.fonts[font].descent  * scale);
    *line_gap = roundf(texture_context.fonts[font].line_gap * scale);
    *location = texture_context.fonts[font].location;
}

void font_char_hmetrics(FontEnum font, i32 font_size, char character, i32* advance, i32* left_side_bearing)
{
    f32 scale = (f32)font_size / texture_context.fonts[font].font_size;
    *advance = roundf(texture_context.fonts[font].chars[character-CHAR_OFFSET].advance * scale);
    *left_side_bearing = roundf(texture_context.fonts[font].chars[character-CHAR_OFFSET].left_side_bearing * scale);
}

void font_char_bbox(FontEnum font, i32 font_size, char character, i32* bbox_x1, i32* bbox_y1, i32* bbox_x2, i32* bbox_y2)
{
    f32 scale = (f32)font_size / texture_context.fonts[font].font_size;
    *bbox_x1 = roundf(texture_context.fonts[font].chars[character-CHAR_OFFSET].x1 * scale);
    *bbox_y1 = roundf(texture_context.fonts[font].chars[character-CHAR_OFFSET].y1 * scale);
    *bbox_x2 = roundf(texture_context.fonts[font].chars[character-CHAR_OFFSET].x2 * scale);
    *bbox_y2 = roundf(texture_context.fonts[font].chars[character-CHAR_OFFSET].y2 * scale);
}

void font_char_bmap(FontEnum font, i32 font_size, char character, f32* bmap_u1, f32* bmap_v1, f32* bmap_u2, f32* bmap_v2)
{
    *bmap_u1 = (f32)(texture_context.fonts[font].chars[character-CHAR_OFFSET].u1) / BITMAP_WIDTH;
    *bmap_v1 = (f32)(texture_context.fonts[font].chars[character-CHAR_OFFSET].v1) / BITMAP_HEIGHT;
    *bmap_u2 = (f32)(texture_context.fonts[font].chars[character-CHAR_OFFSET].u2) / BITMAP_WIDTH;
    *bmap_v2 = (f32)(texture_context.fonts[font].chars[character-CHAR_OFFSET].v2) / BITMAP_HEIGHT;
}

void font_char_kern(FontEnum font, i32 font_size, char character, char next_character, i32* kern)
{
    f32 scale = (f32)font_size / texture_context.fonts[font].font_size;
    *kern = roundf(texture_context.fonts[font].chars[character-CHAR_OFFSET].kern[next_character-CHAR_OFFSET] * scale);
}

static void create_font_textures(i32* tex_unit_location) 
{
    stbtt_pack_context spc;
    unsigned char* bitmap;
    u32 tex;

    bitmap = st_calloc(BITMAP_WIDTH * BITMAP_HEIGHT, sizeof(unsigned char));
    stbtt_PackBegin(&spc, bitmap, BITMAP_WIDTH, BITMAP_HEIGHT, 0, 1, NULL);

    load_font(&spc, FONT_MOJANGLES, 16, "assets/fonts/mojangles.ttf");
    load_font(&spc, FONT_MONOSPACE, 16, "assets/fonts/consola.ttf");

    stbtt_PackEnd(&spc);

    for (i32 font = 0; font < NUM_FONTS; font++)
        texture_context.fonts[font].location = *tex_unit_location;

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, BITMAP_WIDTH, BITMAP_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    i32 swizzle_mask[] = {GL_ZERO, GL_ZERO, GL_ZERO, GL_RED};
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle_mask);

    texture_context.texture_units[*tex_unit_location] = tex;
    glActiveTexture(GL_TEXTURE0 + *tex_unit_location);
    glBindTexture(GL_TEXTURE_2D, texture_context.texture_units[*tex_unit_location]);

    if (1) {
        char path[512];
        sprintf(path, "build/packed_font%d.png", *tex_unit_location);
        stbi_write_png(path, BITMAP_WIDTH, BITMAP_HEIGHT, 1, bitmap, 0);
    }

    st_free(bitmap);
    (*tex_unit_location)++;
}

static void pack_textures(i32* tex_unit_location, unsigned char** image_data, stbrp_rect* rects, i32 num_rects, i32 num_channels)
{
    i32 num_nodes, num_rects_packed;
    i32 y, x, c, data_idx, bitmap_idx;
    i32 location, new_rect_idx;
    i32 all_rects_packed;
    u32 tex;
    unsigned char* bitmap;
    stbrp_context* context;
    stbrp_node* nodes;

    num_nodes = BITMAP_WIDTH;

    context  = st_malloc(sizeof(stbrp_context));
    nodes    = st_malloc(sizeof(stbrp_node) * num_nodes);

    stbrp_init_target(context, BITMAP_WIDTH, BITMAP_HEIGHT, nodes, num_nodes);
    all_rects_packed = stbrp_pack_rects(context, rects, num_rects);

    Texture* texture;
    i32 width, offset_x, offset_y;
    num_rects_packed = 0;
    location = *tex_unit_location;
    bitmap = st_calloc(BITMAP_WIDTH * BITMAP_HEIGHT * num_channels, sizeof(unsigned char));
    for (i32 i = 0; i < num_rects; ++i) {
        stbrp_rect rect = rects[i];
        if (!rect.was_packed)
            continue;
        ++num_rects_packed;
        texture = &texture_context.textures[rect.id];
        offset_x = texture->xint;
        offset_y = texture->yint;
        width = texture->wint;
        for (y = 0; y < rect.h - PADDING; ++y) {
            for (x = 0; x < rect.w - PADDING; ++x) {
                for (c = 0; c < num_channels; ++c) {

                    data_idx   =    (y + offset_y) * num_channels * width
                                 +  (x + offset_x) * num_channels 
                                 +  c;

                    bitmap_idx =   (y + rect.y) * num_channels * BITMAP_WIDTH
                                 + (x + rect.x) * num_channels
                                 +  c;

                    bitmap[bitmap_idx] = image_data[texture->location][data_idx];
                }
            }
        }
    }

    glCreateTextures(GL_TEXTURE_2D, 1, &tex);
    glTextureParameteri(tex, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(tex, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureStorage2D(tex, 1, (num_channels == 3) ? GL_RGB8 : GL_RGBA8, BITMAP_WIDTH, BITMAP_HEIGHT);
    glTextureSubImage2D(tex, 0, 0, 0, BITMAP_WIDTH, BITMAP_HEIGHT, (num_channels == 3) ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, bitmap);
    glActiveTexture(GL_TEXTURE0 + location);
    glBindTexture(GL_TEXTURE_2D, tex);

    texture_context.texture_units[location] = tex;
    new_rect_idx = 0;
    for (i32 i = 0; i < num_rects; ++i) {
        stbrp_rect rect = rects[i];
        if (!rect.was_packed) {
            rects[new_rect_idx++] = rect;
            continue;
        }
        texture_context.textures[rect.id].u = (f32)rect.x / BITMAP_WIDTH;
        texture_context.textures[rect.id].v = (f32)rect.y / BITMAP_HEIGHT;
        texture_context.textures[rect.id].w = (f32)(rect.w-PADDING) / BITMAP_WIDTH;
        texture_context.textures[rect.id].h = (f32)(rect.h-PADDING) / BITMAP_HEIGHT;
        texture_context.textures[rect.id].location = location;
    }

    if (1) {
        char path[512];
        sprintf(path, "build/packed_tex%d.png", *tex_unit_location);
        stbi_write_png(path, BITMAP_WIDTH, BITMAP_HEIGHT, num_channels, bitmap, 0);
    }

    (*tex_unit_location)++;
    st_free(context);
    st_free(nodes);
    st_free(bitmap);

    if (!all_rects_packed) {
        if (*tex_unit_location == NUM_TEXTURE_UNITS) {
            puts("Out of texture units to pack to");
            exit(1);
        }
        pack_textures(tex_unit_location, image_data, rects, new_rect_idx, num_channels);
    }
}

static i32 get_num_textures(JsonObject* json)
{
    JsonIterator* it = json_iterator_create(json);
    JsonMember* member;
    JsonValue* value;
    JsonObject* val_object;
    JsonType type;
    i32 num_members = json_object_length(json);
    assert(it);
    i32 res = 0;
    for (i32 i = 0; i < num_members; i++) {
        member = json_iterator_get(it);
        value = json_member_value(member);
        type = json_get_type(value);

        if (type == JTYPE_STRING)
            res++;
        else if  (type == JTYPE_OBJECT) {
            val_object = json_get_object(value);
            value = json_get_value(val_object, "spritesheet");
            if (value == NULL || json_get_type(value) != JTYPE_TRUE)
                res++;
            else {
                value = json_get_value(val_object, "textures");
                assert(value);
                assert(json_get_type(value) == JTYPE_OBJECT);
                val_object = json_get_object(value);
                res += json_object_length(val_object);
            }
        } else
            exit(1);

        json_iterator_increment(it);
    }

    json_iterator_destroy(it);
    return res;
}

typedef struct {
    vec2 pivot;
    vec2 stretch;
    i32 x, y, w, h;
} ParseObjectArgs;

static i32 get_int(JsonArray* array, i32 idx)
{
    JsonValue* value = json_array_get(array, idx);
    assert(value);
    assert(json_get_type(value) == JTYPE_INT);
    return json_get_int(value);
}

static void parse_array(JsonArray* array, ParseObjectArgs* args)
{
    args->x = get_int(array, 0);
    args->y = get_int(array, 1);
    args->w = get_int(array, 2);
    args->h = get_int(array, 3);
}

static void parse_object(JsonObject* object, ParseObjectArgs* args)
{
    JsonValue* value;
    JsonArray* array;
    value = json_get_value(object, "pivot_x");
    if (value != NULL) {
        assert(json_get_type(value) == JTYPE_FLOAT);
        args->pivot.x = json_get_float(value);
    }
    value = json_get_value(object, "pivot_y");
    if (value != NULL) {
        assert(json_get_type(value) == JTYPE_FLOAT);
        args->pivot.y = json_get_float(value);
    }
    value = json_get_value(object, "stretch_x");
    if (value != NULL) {
        assert(json_get_type(value) == JTYPE_FLOAT);
        args->stretch.x = json_get_float(value);
    }
    value = json_get_value(object, "stretch_y");
    if (value != NULL) {
        assert(json_get_type(value) == JTYPE_FLOAT);
        args->stretch.y = json_get_float(value);
    }
    value = json_get_value(object, "location");
    if (value != NULL) {
        assert(json_get_type(value) == JTYPE_ARRAY);
        array = json_get_array(value);
        assert(array);
        parse_array(array, args);
    }
}

static void parse_texture(JsonMember* member, stbrp_rect* rects, i32 width, i32 height, i32 image_idx, i32* num_rects)
{
    JsonValue* value;
    JsonArray* array;
    JsonObject* val_object;
    JsonType type;
    const char* key;
    char* texture_name;
    key = json_member_key(member);
    assert(key);
    texture_name = string_copy(key);
    value = json_member_value(member);
    assert(value);
    ParseObjectArgs args;
    args.pivot = vec2_create(0, 0);
    args.stretch = vec2_create(1, 1);
    args.x = args.y = 0;
    args.w = width;
    args.h = height;
    texture_context.textures[*num_rects].wint = width;
    type = json_get_type(value);
    if (type == JTYPE_OBJECT) {
        val_object = json_get_object(value);
        parse_object(val_object, &args);
    } else if (type == JTYPE_ARRAY) {
        array = json_get_array(value);
        assert(array);
        parse_array(array, &args);
    }
    rects[*num_rects].id = *num_rects;
    rects[*num_rects].w = PADDING + args.w;
    rects[*num_rects].h = PADDING + args.h;
    texture_context.textures[*num_rects].name = texture_name;
    texture_context.textures[*num_rects].location = image_idx;
    texture_context.textures[*num_rects].pivot = args.pivot;
    texture_context.textures[*num_rects].stretch = args.stretch;
    texture_context.textures[*num_rects].xint = args.x;
    texture_context.textures[*num_rects].yint = args.y;
    (*num_rects)++;
}

static void parse_spritesheet(JsonValue* value, stbrp_rect* rects, i32 width, i32 height, i32 image_idx, i32* num_rects)
{
    i32 num_textures;
    JsonObject* val_object;
    JsonIterator* it;
    JsonMember* member;
    val_object = json_get_object(value);
    value = json_get_value(val_object, "textures");
    assert(value);
    assert(json_get_type(value) == JTYPE_OBJECT);
    val_object = json_get_object(value);
    num_textures = json_object_length(val_object);
    it = json_iterator_create(val_object);
    for (i32 i = 0; i < num_textures; i++) {
        member = json_iterator_get(it);
        parse_texture(member, rects, width, height, image_idx, num_rects);
        json_iterator_increment(it);
    }
    json_iterator_destroy(it);
}

static const char* get_image_path(JsonValue* value, i32* is_spritesheet)
{
    JsonType type;
    JsonObject* val_object;
    const char* image_path = NULL;
    type = json_get_type(value);
    if (type == JTYPE_OBJECT) {
        val_object = json_get_object(value);
        assert(val_object);
        value = json_get_value(val_object, "path");
        assert(value);
        assert(json_get_type(value) == JTYPE_STRING);
        image_path = json_get_string(value);
        value = json_get_value(val_object, "spritesheet");
        if (value != NULL) {
            type = json_get_type(value);
            if (type == JTYPE_TRUE)
                *is_spritesheet = 1;
        }
    } else if (type == JTYPE_STRING)
        image_path = json_get_string(value);
    else
        assert(0);
    return image_path;
}

static void initialize_rects(i32* tex_unit_location)
{
    stbrp_rect* rects;
    unsigned char** image_data;
    i32 width, height, num_channels;
    i32 num_rects;

    const char* textures_path = "config/textures.json";
    JsonObject* json = json_read(textures_path);
    log_assert(json, "Failed to read texture json file %s", textures_path);

    JsonIterator* it = json_iterator_create(json);
    log_assert(it != NULL, "Failed to create json iterator for file %s", textures_path);

    
    JsonMember* member;
    JsonValue* value;
    const char* image_path;
    i32 is_spritesheet;

    i32 num_images = json_object_length(json);
    texture_context.num_textures = get_num_textures(json);

    texture_context.textures = st_malloc(sizeof(Texture) * texture_context.num_textures);
    rects  = st_malloc(sizeof(stbrp_rect) * texture_context.num_textures);
    image_data = st_malloc(sizeof(unsigned char*) * num_images);

    num_rects = 0;
    for (i32 i = 0; i < num_images; i++, json_iterator_increment(it)) {
        is_spritesheet = 0;
        member = json_iterator_get(it);
        value = json_member_value(member);
        image_path = get_image_path(value, &is_spritesheet);
        assert(image_path);

        image_data[i] = stbi_load(image_path, &width, &height, &num_channels, 4);
        if (image_data[i] == NULL) {
            log_write(WARNING, "Could not open %s", image_path);
            width = height = 0;
        }

        if (!is_spritesheet)
            parse_texture(member, rects, width, height, i, &num_rects);
        else
            parse_spritesheet(value, rects, width, height, i, &num_rects);
    }

    pack_textures(tex_unit_location, image_data, rects, num_rects, 4);

    qsort(texture_context.textures, texture_context.num_textures, sizeof(Texture), texture_cmp);

    for (i32 i = 0; i < num_images; i++)
        stbi_image_free(image_data[i]);

    json_iterator_destroy(it);
    json_object_destroy(json);
    st_free(rects);
    st_free(image_data);
}

i32 texture_get_id(const char* name)
{
    i32 l, m, r, a;
    l = 0, r = texture_context.num_textures - 1;
    while (l <= r) {
        m = l + (r - l) / 2;
        a = strcmp(name, texture_context.textures[m].name);
        if (a > 0)
            l = m + 1;
        else if (a < 0)
            r = m - 1;
        else
            return m;
    }
    log_write(FATAL, "Failed to get id for %s\n", name);
    return -1;
}

void texture_info(i32 id, i32* location, f32* u, f32* v, f32* w, f32* h, vec2* pivot, vec2* stretch)
{
    *location = texture_context.textures[id].location;
    *u = texture_context.textures[id].u;
    *v = texture_context.textures[id].v;
    *w = texture_context.textures[id].w;
    *h = texture_context.textures[id].h;
    *pivot = texture_context.textures[id].pivot;
    *stretch = texture_context.textures[id].stretch;
}

GLuint texture_get_unit(TextureEnum tex)
{
    return texture_context.static_textures[tex].unit;
}

GLuint texture_get_name(TextureEnum tex)
{
    return texture_context.static_textures[tex].name;
}

static void create_static_textures(i32* tex_unit_location)
{
    if (*tex_unit_location + NUM_TEXTURES >= NUM_TEXTURE_UNITS)
        log_write(FATAL, "Out of texture units");
    for (i32 i = 0; i < NUM_TEXTURES; i++) {
        glGenTextures(1, &texture_context.static_textures[i].name);
        texture_context.static_textures[i].unit = *tex_unit_location;
        texture_context.texture_units[*tex_unit_location] = texture_context.static_textures[i].name;
        glActiveTexture(GL_TEXTURE0 + (*tex_unit_location)++);
        glBindTexture(GL_TEXTURE_2D, texture_context.static_textures[i].name);
    }
}

void texture_init(void)
{
    i32 tex_unit_location;
    tex_unit_location = 0;
    create_static_textures(&tex_unit_location);
    create_font_textures(&tex_unit_location);
    initialize_rects(&tex_unit_location);
}

void texture_cleanup(void)
{
    for (i32 i = 0; i < texture_context.num_textures; i++)
        st_free(texture_context.textures[i].name);
    st_free(texture_context.textures);
    for (i32 i = 0; i < NUM_TEXTURE_UNITS; i++)
        if (texture_context.texture_units[i] != 0)
            glDeleteTextures(1, &texture_context.texture_units[i]);
}

