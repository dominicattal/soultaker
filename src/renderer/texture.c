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
    TextureEnum tex;
    char* path;
} Image;

typedef struct {
    f32 u, v, w, h;
    i32 location;
} Texture;

typedef struct {
    Font fonts[NUM_FONTS];
    Texture* textures;
    u32 texture_units[NUM_TEXTURE_UNITS];
} TextureContext;

#define NUM_IMAGES_TO_PACK (int)(sizeof(images) / sizeof(Image))
static Image images[] = {
    (Image) {TEX_NONE, "assets/textures/none.png"},
    (Image) {TEX_COLOR, "assets/textures/color.png"},
    (Image) {TEX_TILE_1, "assets/textures/tile_1.png"},
    (Image) {TEX_TILE_2, "assets/textures/tile_2.png"},
    (Image) {TEX_WALL_1, "assets/textures/wall_top.png"},
    (Image) {TEX_WALL_2, "assets/textures/wall_side.png"},
    (Image) {TEX_KNIGHT_IDLE_DOWN, "assets/textures/knight/knight_idle_down.png"},
    (Image) {TEX_KNIGHT_IDLE_UP, "assets/textures/knight/knight_idle_up.png"},
    (Image) {TEX_KNIGHT_IDLE_RIGHT, "assets/textures/knight/knight_idle_right.png"},
    (Image) {TEX_KNIGHT_IDLE_LEFT, "assets/textures/knight/knight_idle_left.png"},
    (Image) {TEX_KNIGHT_WALKING_DOWN_1, "assets/textures/knight/knight_walk_down_1.png"},
    (Image) {TEX_KNIGHT_WALKING_DOWN_2, "assets/textures/knight/knight_walk_down_2.png"},
    (Image) {TEX_KNIGHT_WALKING_UP_1, "assets/textures/knight/knight_walk_up_1.png"},
    (Image) {TEX_KNIGHT_WALKING_UP_2, "assets/textures/knight/knight_walk_up_2.png"},
    (Image) {TEX_KNIGHT_WALKING_RIGHT, "assets/textures/knight/knight_walk_right.png"},
    (Image) {TEX_KNIGHT_WALKING_LEFT, "assets/textures/knight/knight_walk_left.png"},
    (Image) {TEX_KNIGHT_SHOOTING_DOWN_1, "assets/textures/knight/knight_shoot_down_1.png"},
    (Image) {TEX_KNIGHT_SHOOTING_UP_1, "assets/textures/knight/knight_shoot_up_1.png"},
    (Image) {TEX_KNIGHT_SHOOTING_RIGHT_1, "assets/textures/knight/knight_shoot_right_1.png"},
    (Image) {TEX_KNIGHT_SHOOTING_LEFT_1, "assets/textures/knight/knight_shoot_left_1.png"},
    (Image) {TEX_KNIGHT_SHOOTING_DOWN_2, "assets/textures/knight/knight_shoot_down_2.png"},
    (Image) {TEX_KNIGHT_SHOOTING_UP_2, "assets/textures/knight/knight_shoot_up_2.png"},
    (Image) {TEX_KNIGHT_SHOOTING_RIGHT_2, "assets/textures/knight/knight_shoot_right_2.png"},
    (Image) {TEX_KNIGHT_SHOOTING_LEFT_2, "assets/textures/knight/knight_shoot_left_2.png"},
    (Image) {TEX_ROCK, "assets/textures/rock.png"},
    (Image) {TEX_BUSH, "assets/textures/bush.png"},
    (Image) {TEX_BULLET, "assets/textures/bullet.png"}
};

static TextureContext ctx;

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
    font_buffer = malloc(size);
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
    stbtt_GetFontVMetrics(&info, &ctx.fonts[font].ascent, &ctx.fonts[font].descent, &ctx.fonts[font].line_gap);
    ctx.fonts[font].ascent = roundf(ctx.fonts[font].ascent * scale);
    ctx.fonts[font].descent = roundf(ctx.fonts[font].descent * scale);
    ctx.fonts[font].line_gap = roundf(ctx.fonts[font].line_gap * scale);
    ctx.fonts[font].font_size = font_size;
    for (i32 i = 0; i < NUM_CHARS; i++) {
        ctx.fonts[font].chars[i].u1 = chars[i].x0;
        ctx.fonts[font].chars[i].v1 = chars[i].y0;
        ctx.fonts[font].chars[i].u2 = chars[i].x1;
        ctx.fonts[font].chars[i].v2 = chars[i].y1;
        stbtt_GetCodepointHMetrics(&info, i+CHAR_OFFSET, 
                                   &ctx.fonts[font].chars[i].advance, 
                                   &ctx.fonts[font].chars[i].left_side_bearing);
        ctx.fonts[font].chars[i].advance = roundf(ctx.fonts[font].chars[i].advance * scale);
        ctx.fonts[font].chars[i].left_side_bearing = roundf(ctx.fonts[font].chars[i].left_side_bearing * scale);
        stbtt_GetCodepointBitmapBox(&info, i+CHAR_OFFSET, scale, scale, 
                                    &ctx.fonts[font].chars[i].x1,  
                                    &ctx.fonts[font].chars[i].y1,  
                                    &ctx.fonts[font].chars[i].x2,  
                                    &ctx.fonts[font].chars[i].y2);
        for (i32 j = 0; j < NUM_CHARS; j++) {
            ctx.fonts[font].chars[i].kern[j] = stbtt_GetCodepointKernAdvance(&info, i+CHAR_OFFSET, j+CHAR_OFFSET);
            ctx.fonts[font].chars[i].kern[j] = roundf(ctx.fonts[font].chars[i].kern[j] * scale);
        }
    }

    free(font_buffer);
}

void font_info(FontEnum font, i32 font_size, i32* ascent, i32* descent, i32* line_gap, i32* location)
{
    f32 scale = (f32)font_size / ctx.fonts[font].font_size;
    *ascent   = roundf(ctx.fonts[font].ascent   * scale);
    *descent  = roundf(ctx.fonts[font].descent  * scale);
    *line_gap = roundf(ctx.fonts[font].line_gap * scale);
    *location = ctx.fonts[font].location;
}

void font_char_hmetrics(FontEnum font, i32 font_size, char character, i32* advance, i32* left_side_bearing)
{
    f32 scale = (f32)font_size / ctx.fonts[font].font_size;
    *advance = roundf(ctx.fonts[font].chars[character-CHAR_OFFSET].advance * scale);
    *left_side_bearing = roundf(ctx.fonts[font].chars[character-CHAR_OFFSET].left_side_bearing * scale);
}

void font_char_bbox(FontEnum font, i32 font_size, char character, i32* bbox_x1, i32* bbox_y1, i32* bbox_x2, i32* bbox_y2)
{
    f32 scale = (f32)font_size / ctx.fonts[font].font_size;
    *bbox_x1 = roundf(ctx.fonts[font].chars[character-CHAR_OFFSET].x1 * scale);
    *bbox_y1 = roundf(ctx.fonts[font].chars[character-CHAR_OFFSET].y1 * scale);
    *bbox_x2 = roundf(ctx.fonts[font].chars[character-CHAR_OFFSET].x2 * scale);
    *bbox_y2 = roundf(ctx.fonts[font].chars[character-CHAR_OFFSET].y2 * scale);
}

void font_char_bmap(FontEnum font, i32 font_size, char character, f32* bmap_u1, f32* bmap_v1, f32* bmap_u2, f32* bmap_v2)
{
    *bmap_u1 = (f32)(ctx.fonts[font].chars[character-CHAR_OFFSET].u1) / BITMAP_WIDTH;
    *bmap_v1 = (f32)(ctx.fonts[font].chars[character-CHAR_OFFSET].v1) / BITMAP_HEIGHT;
    *bmap_u2 = (f32)(ctx.fonts[font].chars[character-CHAR_OFFSET].u2) / BITMAP_WIDTH;
    *bmap_v2 = (f32)(ctx.fonts[font].chars[character-CHAR_OFFSET].v2) / BITMAP_HEIGHT;
}

void font_char_kern(FontEnum font, i32 font_size, char character, char next_character, i32* kern)
{
    f32 scale = (f32)font_size / ctx.fonts[font].font_size;
    *kern = roundf(ctx.fonts[font].chars[character-CHAR_OFFSET].kern[next_character-CHAR_OFFSET] * scale);
}

static void create_font_textures(i32* tex_unit_location) 
{
    stbtt_pack_context spc;
    unsigned char* bitmap;
    u32 tex;

    bitmap = calloc(BITMAP_WIDTH * BITMAP_HEIGHT, sizeof(unsigned char));
    stbtt_PackBegin(&spc, bitmap, BITMAP_WIDTH, BITMAP_HEIGHT, 0, 1, NULL);

    load_font(&spc, FONT_MOJANGLES, 16, "assets/fonts/mojangles.ttf");
    load_font(&spc, FONT_MONOSPACE, 16, "assets/fonts/consola.ttf");

    stbtt_PackEnd(&spc);

    for (i32 font = 0; font < NUM_FONTS; font++)
        ctx.fonts[font].location = *tex_unit_location;

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, BITMAP_WIDTH, BITMAP_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    i32 swizzle_mask[] = {GL_ZERO, GL_ZERO, GL_ZERO, GL_RED};
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle_mask);

    ctx.texture_units[*tex_unit_location] = tex;
    glActiveTexture(GL_TEXTURE0 + *tex_unit_location);
    glBindTexture(GL_TEXTURE_2D, ctx.texture_units[*tex_unit_location]);

    if (1) {
        char path[512];
        sprintf(path, "build/packed_font%d.png", *tex_unit_location);
        stbi_write_png(path, BITMAP_WIDTH, BITMAP_HEIGHT, 1, bitmap, 0);
    }

    free(bitmap);
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

    context  = malloc(sizeof(stbrp_context));
    nodes    = malloc(sizeof(stbrp_node) * num_nodes);

    stbrp_init_target(context, BITMAP_WIDTH, BITMAP_HEIGHT, nodes, num_nodes);
    all_rects_packed = stbrp_pack_rects(context, rects, num_rects);

    num_rects_packed = 0;
    location = *tex_unit_location;
    bitmap = calloc(BITMAP_WIDTH * BITMAP_HEIGHT * num_channels, sizeof(unsigned char));
    for (i32 i = 0; i < num_rects; ++i) {
        if (!rects[i].was_packed)
            continue;
        ++num_rects_packed;
        for (y = 0; y < rects[i].h - PADDING; ++y) {
            for (x = 0; x < rects[i].w - PADDING; ++x) {
                for (c = 0; c < num_channels; ++c) {
                    stbrp_rect rect = rects[i];

                    data_idx   =    y * num_channels * (rect.w - PADDING)
                                 +  x * num_channels 
                                 +  c;

                    bitmap_idx =   (y + rect.y) * num_channels * BITMAP_WIDTH
                                 + (x + rect.x) * num_channels
                                 +  c;

                    bitmap[bitmap_idx] = image_data[ctx.textures[rect.id].location][data_idx];
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

    ctx.texture_units[location] = tex;
    new_rect_idx = 0;
    for (i32 i = 0; i < num_rects; ++i) {
        stbrp_rect rect = rects[i];
        if (!rect.was_packed) {
            rects[new_rect_idx++] = rect;
            continue;
        }
        ctx.textures[rect.id].u = (f32)rect.x / BITMAP_WIDTH;
        ctx.textures[rect.id].v = (f32)rect.y / BITMAP_HEIGHT;
        ctx.textures[rect.id].w = (f32)(rect.w-PADDING) / BITMAP_WIDTH;
        ctx.textures[rect.id].h = (f32)(rect.h-PADDING) / BITMAP_HEIGHT;
        ctx.textures[rect.id].location = location;
    }

    if (1) {
        char path[512];
        sprintf(path, "build/packed_tex%d.png", *tex_unit_location);
        stbi_write_png(path, BITMAP_WIDTH, BITMAP_HEIGHT, num_channels, bitmap, 0);
    }

    (*tex_unit_location)++;
    free(context);
    free(nodes);
    free(bitmap);

    if (!all_rects_packed) {
        if (*tex_unit_location == NUM_TEXTURE_UNITS) {
            puts("Out of texture units to pack to");
            exit(1);
        }
        pack_textures(tex_unit_location, image_data, rects, new_rect_idx, num_channels);
    }
}

static void initialize_rects(i32* tex_unit_location)
{
    stbrp_rect* rects;
    unsigned char** image_data;
    i32 width, height, num_channels;
    i32 num_rects;

    JsonObject* json = json_read("assets/config/textures.json");
    assert(json != NULL);

    int num_textures = json_object_length(json);

    ctx.textures = malloc(NUM_TEXTURES * sizeof(Texture));
    rects  = malloc(sizeof(stbrp_rect) * NUM_IMAGES_TO_PACK);
    image_data = malloc(sizeof(unsigned char*) * NUM_IMAGES_TO_PACK);

    num_rects = 0;
    for (i32 i = 0; i < NUM_IMAGES_TO_PACK; i++) {
        image_data[i] = stbi_load(images[i].path, &width, &height, &num_channels, 4);
        if (image_data[i] == NULL) {
            printf("Could not open %s\n", images[i].path);
            continue;
        }
        rects[num_rects].id = images[i].tex;
        rects[num_rects].w = PADDING + width;
        rects[num_rects].h = PADDING + height;
        ctx.textures[images[i].tex].location = i;
        num_rects++;
    }

    pack_textures(tex_unit_location, image_data, rects, num_rects, 4);

    for (i32 i = 0; i < NUM_IMAGES_TO_PACK; i++)
        stbi_image_free(image_data[i]);

    json_object_destroy(json);
    free(rects);
    free(image_data);
}

void texture_info(TextureEnum tex, f32* u, f32* v, f32* w, f32* h, i32* location)
{
    *u = ctx.textures[tex].u;
    *v = ctx.textures[tex].v;
    *w = ctx.textures[tex].w;
    *h = ctx.textures[tex].h;
    *location = ctx.textures[tex].location;
}

void texture_init(void)
{
    i32 tex_unit_location;
    tex_unit_location = 0;
    create_font_textures(&tex_unit_location);
    initialize_rects(&tex_unit_location);
}

void texture_cleanup(void)
{
    free(ctx.textures);
    glDeleteTextures(NUM_TEXTURE_UNITS, ctx.texture_units);
}

