#ifndef RENDERER_H
#define RENDERER_H

#include "util.h"

#define NUM_TEXTURE_UNITS 16

typedef enum {
    SHADER_PROGRAM_GUI,
    SHADER_PROGRAM_TILE,
    SHADER_PROGRAM_ENTITY,
    SHADER_PROGRAM_ENTITY_COMP,
    NUM_SHADER_PROGRAMS
} ShaderProgramEnum;

typedef enum {
    TEX_NONE,
    TEX_COLOR,
    TEX_TILE_1,
    TEX_TILE_2,
    TEX_KNIGHT,
    NUM_TEXTURES
} TextureEnum;

typedef enum {
    TEX_ATLAS_FONT_STATIC,
    TEX_ATLAS_GUI_STATIC,
    TEX_ATLAS_GAME_STATIC,
    NUM_TEXTURE_ATLASES
} TextureAtlasEnum;

typedef enum {
    FONT_MONOSPACE,
    FONT_MOJANGLES,
    NUM_FONTS
} FontEnum;

typedef enum {
    UBO_INDEX_DEFAULT,
    UBO_INDEX_WINDOW,
    UBO_INDEX_MATRICES
} UBOIndexEnum;

void renderer_init(void);
void renderer_render(void);
void renderer_cleanup(void);
GLint renderer_get_major_version(void);
GLint renderer_get_minor_version(void);
const char* renderer_get_version(void);
const char* renderer_get_vendor(void);
const char* renderer_get_renderer_name(void);
void renderer_list_available_extensions(void);
void renderer_list_available_versions(void);
void renderer_list_context_flags(void);
void renderer_print_context_profile(void);

void shader_init(void);
void shader_compile(ShaderProgramEnum program);
void shader_use(ShaderProgramEnum program);
GLuint shader_get_uniform_location(ShaderProgramEnum program, const char* identifier);
void shader_bind_uniform_block(ShaderProgramEnum program, u32 index, const char* identifier);
void shader_cleanup(void);

void texture_init(void);
void texture_info(TextureEnum tex, f32* u, f32* v, f32* w, f32* h, i32* location);
void texture_atlas_load(TextureAtlasEnum atlas);
void texture_atlas_unload(TextureAtlasEnum atlas);
void texture_cleanup(void);

void font_info(FontEnum font, i32 font_size, i32* ascent, i32* descent, i32* line_gap, i32* location);
void font_char_hmetrics(FontEnum font, i32 font_size, char character, i32* advance, i32* left_side_bearing);
void font_char_bbox(FontEnum font, i32 font_size, char character, i32* bbox_x1, i32* bbox_y1, i32* bbox_x2, i32* bbox_y2);
void font_char_bmap(FontEnum font, i32 font_size, char character, f32* bmap_u1, f32* bmap_v1, f32* bmap_u2, f32* bmap_v2);
void font_char_kern(FontEnum font, i32 font_size, char character, char next_character, i32* kern);

#endif
