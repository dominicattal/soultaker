#ifndef RENDERER_H
#define RENDERER_H

#include "util.h"

typedef struct {
    int x;
} RenderContext;

extern RenderContext render_context;

typedef enum {
    SHADER_PROGRAM_GUI,
    SHADER_PROGRAM_GAME,
    NUM_SHADER_PROGRAMS
} ShaderProgram;

typedef enum {
    TEX_NONE,
    TEX_COLOR,
    NUM_TEXTURES
} Texture;

typedef enum {
    FONT_MONOSPACE,
    FONT_MOJANGLES,
    NUM_FONTS
} Font;

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
void shader_cleanup(void);

void texture_init(void);
void texture_cleanup(void);

void renderer_use_shader_program(ShaderProgram program);

#endif
