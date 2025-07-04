#include "../gui.h"
#include "../game.h"

static void GLAPIENTRY message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
        return;

    char* source_str;
    char* type_str;
    char* severity_str;
    LogLevel level;
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
            severity_str = "GL_DEBUG_SEVERITY_NOTIFICATION"; 
            level = INFO;
            break;
        case GL_DEBUG_SEVERITY_LOW:
            severity_str = "GL_DEBUG_SEVERITY_LOW"; 
            level = WARNING;
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            severity_str = "GL_DEBUG_SEVERITY_MEDIUM"; 
            level = ERROR;
            break;
        case GL_DEBUG_SEVERITY_HIGH:
            severity_str = "GL_DEBUG_SEVERITY_HIGH"; 
            level = FATAL;
            break;
        default:
            severity_str = ""; 
            level = INFO;
            break;
    }
    log_write(level, "OpenGL error\n"
                     "%-8s = 0x%04x %s\n"
                     "%-8s = 0x%04x %s\n"
                     "%-8s = 0x%04x %s\n"
                     "%-8s = %u\n"
                     "%s",
                      "source", source, source_str,
                      "type", type, type_str,
                      "severity", severity, severity_str,
                      "id", id, message);
}

void renderer_init(void)
{
    log_write(INFO, "Initializing renderer...");
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(message_callback, 0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glEnable(GL_CULL_FACE);
    shader_init();
    texture_init();
    log_write(INFO, "Renderer initialized");
}

void renderer_render(void)
{
    //glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glClearColor(0.4f, 0.4f, 0.4f, 1.0f);

    game_render();
    gui_render();
}

void renderer_cleanup(void)
{
    log_write(INFO, "Cleaning up renderer...");
    shader_cleanup();
    texture_cleanup();
    log_write(INFO, "Cleaned up renderer");
}

