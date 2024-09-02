#ifndef FBO_H
#define FBO_H

#include <glad.h>
#include <glfw.h>
#include "../../util/type.h"
#include "../rbo/rbo.h"

typedef struct {
    u32 id, color_buffer_id;
    RBO rbo;
} FBO;

FBO fbo_create(u32 width, u32 height);
void fbo_bind(FBO fbo);
void fbo_bind_color_buffer(FBO fbo);
void fbo_bind_default(void);
void fbo_attach_rbo(FBO fbo, RBO rbo);
bool fbo_check_status(FBO fbo);
void fbo_destroy(FBO fbo);

#endif