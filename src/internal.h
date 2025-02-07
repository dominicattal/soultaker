#ifndef RENDERER_INTERNAL_H
#define RENDERER_INTERNAL_H

#include "../renderer.h"

typedef struct {
    ShaderProgram shader_programs[NUM_SHADER_PROGRAMS];    
} RenderContext;

extern RenderContext render_context;

#endif
