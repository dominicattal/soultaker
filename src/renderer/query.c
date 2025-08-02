#include "internal.h"

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

void renderer_print_context_profile(void)
{
    GLint mask;
    glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &mask);
    printf((mask == GL_CONTEXT_CORE_PROFILE_BIT) ? "GL_CONTEXT_CORE_PROFILE\n" : "GL_CONTEXT_COMPATIBILITY_PROFILE\n");
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
