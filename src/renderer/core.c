#include "../renderer.h"

RenderContext render_context;

void renderer_init(void)
{
    renderer_print_context_profile();
    renderer_list_context_flags();
}

void renderer_cleanup(void)
{
}


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