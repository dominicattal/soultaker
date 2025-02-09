#ifndef RENDERER_H
#define RENDERER_H

#include "util.h"

#define NUM_TEXTURE_UNITS 16

typedef enum {
    SHADER_PROGRAM_GUI,
    SHADER_PROGRAM_GAME,
    NUM_SHADER_PROGRAMS
} ShaderProgramEnum;

typedef enum {
    TEX_NONE,
    TEX_COLOR,
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
void texture_info(TextureEnum tex, f32* u1, f32* u2, f32* v1, f32* v2, u32* location);
void texture_atlas_load(TextureAtlasEnum atlas);
void texture_atlas_unload(TextureAtlasEnum atlas);
void texture_cleanup(void);

#endif
