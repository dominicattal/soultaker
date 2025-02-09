#include "internal.h"
#include "../gui.h"
#include "../game.h"

RenderContext render_context;

static void GLAPIENTRY message_callback(GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar*, const void*);

void renderer_init(void)
{
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(message_callback, 0);
    shader_init();
}

void renderer_render(void)
{
    //glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.5f, 0.5f, 0.7f, 1.0f);

    gui_render();
    game_render();
}

void renderer_cleanup(void)
{
    shader_cleanup();
}

static void GLAPIENTRY message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
        return;

    puts("======== gl message callback ========");
    char* source_str;
    char* type_str;
    char* severity_str;
    switch (source) {
        case GL_DEBUG_SOURCE_API:
            source_str = "GL_DEBUG_SOURCE_API"; break;
        default:
            source_str = ""; break;
    }
    switch (type) {
        case GL_DEBUG_TYPE_OTHER:
            type_str = "GL_DEBUG_TYPE_OTHER"; break;
        case GL_DEBUG_TYPE_ERROR:
            type_str = "GL_DEBUG_TYPE_ERROR"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            type_str = "GL_DEBUG_TYPE_PERFORMANCE"; break;
        default:
            type_str = ""; break;
    }
    switch (severity) {
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            severity_str = "GL_DEBUG_SEVERITY_NOTIFICATION"; break;
        case GL_DEBUG_SEVERITY_LOW:
            severity_str = "GL_DEBUG_SEVERITY_LOW"; break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            severity_str = "GL_DEBUG_SEVERITY_MEDIUM"; break;
        case GL_DEBUG_SEVERITY_HIGH:
            severity_str = "GL_DEBUG_SEVERITY_HIGH"; break;
        default:
            severity_str = ""; break;
    }
    printf("%-8s = 0x%04x %s\n", "source", source, source_str);
    printf("%-8s = 0x%04x %s\n", "type", type, type_str);
    printf("%-8s = 0x%04x %s\n", "severity", severity, severity_str);
    printf("%-8s = %u\n\n", "id", id);
    printf("%s\n\n", message);
    
    if (severity != GL_DEBUG_SEVERITY_NOTIFICATION)
        exit(1);
}
