#ifndef RENDERER_INTERNAL_H
#define RENDERER_INTERNAL_H

#include "../renderer.h"
#include "../util.h"

typedef struct {
    GLuint shader_programs[NUM_SHADER_PROGRAMS];
} RenderContext;

extern RenderContext rctx;

#endif
