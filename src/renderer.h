#ifndef RENDERER_H
#define RENDERER_H

#include "util.h"

#define NUM_TEXTURE_UNITS 16

typedef enum {
    SHADER_PROGRAM_NONE,
    SHADER_PROGRAM_SCREEN,
    SHADER_PROGRAM_GUI,
    SHADER_PROGRAM_TILE,
    SHADER_PROGRAM_WALL,
    SHADER_PROGRAM_ENTITY,
    SHADER_PROGRAM_PROJECTILE,
    SHADER_PROGRAM_OBSTACLE,
    SHADER_PROGRAM_PARTICLE,
    SHADER_PROGRAM_PARJICLE,
    SHADER_PROGRAM_SHADOW,
    SHADER_PROGRAM_MINIMAP_CIRCLE,
    NUM_SHADER_PROGRAMS
} ShaderProgramEnum;

typedef enum {
    TEX_GAME_SCENE,
    TEX_GAME_SHADOW_SCENE,
    TEX_GAME_MINIMAP_SCENE,
    NUM_STATIC_TEXTURES
} TextureEnum;

typedef enum {
    FONT_MONOSPACE,
    FONT_MOJANGLES,
    NUM_FONTS
} FontEnum;

typedef enum {
    UBO_INDEX_DEFAULT,
    UBO_INDEX_WINDOW,
    UBO_INDEX_MATRICES,
    UBO_INDEX_GAME_TIME,
    UBO_INDEX_MINIMAP
} UBOIndexEnum;

void renderer_init(void);
void renderer_render(void);
void renderer_cleanup(void);
GLint renderer_get_major_version(void);
GLint renderer_get_minor_version(void);
const char* renderer_get_version(void);
const char* renderer_get_vendor(void);
const char* renderer_get_renderer_name(void);
GLint renderer_get_max_image_units(void);
void renderer_list_available_extensions(void);
void renderer_list_available_versions(void);
void renderer_list_context_flags(void);
void renderer_print_context_profile(void);
void renderer_print_state(void);

// converts active texture units to png
void renderer_write_texture_unit(i32 unit);
void renderer_write_texture_units(void);

// checks if framebuffer is complete, prints error and exits if not
void renderer_check_framebuffer_status(GLenum target, const char* name);

void shader_init(void);
void shader_compile(ShaderProgramEnum program);
void shader_use(ShaderProgramEnum program);
GLuint shader_get_uniform_location(ShaderProgramEnum program, const char* identifier);
void shader_bind_uniform_block(ShaderProgramEnum program, u32 index, const char* identifier);
void shader_cleanup(void);

void texture_init(void);
GLuint texture_get_unit(TextureEnum tex);
GLuint texture_get_name(TextureEnum tex);
i32 texture_get_enum_id(TextureEnum tex);
i32 texture_get_id(const char* handle);
void texture_set_dimensions(i32 id, f32 u, f32 v, f32 w, f32 h);
void texture_info(i32 id, i32* location, f32* u, f32* v, f32* w, f32* h, vec2* pivot, vec2* stretch);
void texture_cleanup(void);

void font_info(FontEnum font, i32 font_size, i32* ascent, i32* descent, i32* line_gap, i32* location);
void font_char_hmetrics(FontEnum font, i32 font_size, char character, i32* advance, i32* left_side_bearing);
void font_char_bbox(FontEnum font, i32 font_size, char character, i32* bbox_x1, i32* bbox_y1, i32* bbox_x2, i32* bbox_y2);
void font_char_bmap(FontEnum font, i32 font_size, char character, f32* bmap_u1, f32* bmap_v1, f32* bmap_u2, f32* bmap_v2);
void font_char_kern(FontEnum font, i32 font_size, char character, char next_character, i32* kern);

#endif
