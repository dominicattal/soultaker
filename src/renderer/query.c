#include "internal.h"
#include <string.h>
#include <stb_image_write.h>

GLint renderer_get_major_version(void)
{
    GLint major_version;
    glGetIntegerv(GL_MAJOR_VERSION, &major_version);
    return major_version;
}

GLint renderer_get_minor_version(void)
{
    GLint minor_version;
    glGetIntegerv(GL_MINOR_VERSION, &minor_version);
    return minor_version;
}

const char* renderer_get_version(void)
{
    return (const char*)glGetString(GL_VERSION);
}

const char* renderer_get_vendor(void)
{
    return (const char*)glGetString(GL_RENDERER);
}

const char* renderer_get_renderer_name(void)
{
    return (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
}

GLint renderer_get_max_image_units(void)
{
    GLint res;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &res);
    return res;
}

void renderer_print_context_profile(void)
{
    GLint mask;
    glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &mask);
    printf((mask == GL_CONTEXT_CORE_PROFILE_BIT) ? "GL_CONTEXT_CORE_PROFILE\n" : "GL_CONTEXT_COMPATIBILITY_PROFILE\n");
}

static u8* get_pixels(i32 w, i32 h, i32 fmt, i32* c)
{
    i32 length;
    u8* pixels = NULL;
    GLenum format;
    switch (fmt) {
        case GL_RGBA8:
            *c = 4;
            format = GL_RGBA;
            break;
        case GL_RGB:
            *c = 3;
            format = GL_RGB;
            break;
        case GL_RED:
            *c = 1;
            format = GL_RED;
            break;
        default:
            return NULL;
    }
    length = (*c) * w * h;
    pixels = st_malloc(length * sizeof(u8));
    glGetTexImage(GL_TEXTURE_2D, 0, format, GL_UNSIGNED_BYTE, pixels);
    return pixels;
}

void renderer_write_texture_unit(i32 unit)
{
    i32 w, h, fmt, c;
    i32 max_image_units = renderer_get_max_image_units();
    if (strcmp(thread_get_self_name(), "Main") != 0) {
        log_write(DEBUG, "not queried from opengl context thread");
        return;
    }
    if (unit >= max_image_units) {
        log_write(DEBUG, "unit %d out of range (max=%d)", unit, max_image_units);
        return;
    }
    glActiveTexture(GL_TEXTURE0 + unit);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &fmt);
    if (w == 0 || h == 0) {
        log_write(DEBUG, "unit %d is not active", unit);
        return;
    }
    u8* pixels = get_pixels(w, h, fmt, &c);
    if (pixels == NULL) {
        log_write(DEBUG, "unit %d failed to get pixels", unit);
        return;
    }
    char* path = string_create("data/unit%d.png", 100, unit);
    stbi_write_png(path, w, h, c, pixels, 0);
    string_free(path);
    st_free(pixels);

    log_write(DEBUG, "unit=%-2d w=%-4d h=%-4d fmt=0x%x", unit, w, h, fmt);
}

void renderer_write_texture_units(void)
{
    i32 i, w, h, fmt, c, n;
    i32 max_image_units = renderer_get_max_image_units();
    char* summary = string_create("summary", 10);
    char* line;
    char* tmp;
    log_write(DEBUG, "max_image_units=%d", max_image_units);
    for (i = 0; i < 32; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &fmt);
        if (w == 0 || h == 0)
            continue;
        u8* pixels = get_pixels(w, h, fmt, &c);
        if (pixels == NULL)
            continue;
        char* path = string_create("data/unit%d.png", 100, i);
        stbi_write_png(path, w, h, c, pixels, 0);
        line = string_create("unit=%-2d w=%-4d h=%-4d fmt=0x%x", 100, i, w, h, fmt);
        tmp = summary;
        n = strlen(line) + strlen(tmp) + 1;
        summary = string_create("%s\n%s", n, summary, line);

        string_free(path);
        string_free(tmp);
        string_free(line);
        st_free(pixels);
    }
    log_write(DEBUG, summary);
    string_free(summary);
}

void renderer_list_available_extensions(void)
{
    GLint num_extensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
    for (i32 i = 0; i < num_extensions; i++)
        printf("%s\n", (const char*)glGetStringi(GL_EXTENSIONS, i));
}

void renderer_list_available_versions(void)
{
    GLint num_versions;
    glGetIntegerv(GL_NUM_SHADING_LANGUAGE_VERSIONS , &num_versions);
    for (i32 i = 0; i < num_versions; i++)
        printf("%s\n", (const char*)glGetStringi(GL_SHADING_LANGUAGE_VERSION, i));
}

void renderer_list_context_flags(void)
{
    GLint mask;
    glGetIntegerv(GL_CONTEXT_FLAGS, &mask);
    printf("%sGL_CONTEXT_FLAG_FORWARD_COMPATIBLE\n", (mask & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT) ? "" : "NOT ");
    printf("%sGL_CONTEXT_FLAG_DEBUG\n", (mask & GL_CONTEXT_FLAG_DEBUG_BIT) ? "" : "NOT ");
    printf("%sGL_CONTEXT_FLAG_ROBUST_ACCESS\n", (mask & GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT) ? "" : "NOT ");
    printf("%sGL_CONTEXT_FLAG_NO_ERROR\n", (mask & GL_CONTEXT_FLAG_NO_ERROR_BIT) ? "" : "NOT ");
}

void renderer_check_framebuffer_status(GLenum target, const char* name)
{
    GLenum status = glCheckFramebufferStatus(target);
    const char* message = "";
    if (status == GL_FRAMEBUFFER_COMPLETE)
        return;
    switch (status) {
        case GL_FRAMEBUFFER_UNDEFINED:
            message = "the specified framebuffer is the default read or draw framebuffer, but the default framebuffer does not exist.";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            message = "one of the framebuffer attachment points are framebuffer incomplete.";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            message = "the framebuffer does not have at least one image attached to it.";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            message = "the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for any color attachment point(s) named by GL_DRAW_BUFFERi";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            message = "GL_READ_BUFFER is not GL_NONE and the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for the color attachment point named by GL_READ_BUFFER.";
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            message = "the combination of internal formats of the attached images violates an implementation-dependent set of restrictions.";
            break;
        default:
            message = "unknown";
            break;
    }
    log_write(FATAL, "%s framebuffer error\nGLenum: %04X\n%s", name, status, message);
}
