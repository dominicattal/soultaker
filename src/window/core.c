#include "internal.h"

#define DEFAULT_WINDOW_WIDTH 1000
#define DEFAULT_WINDOW_HEIGHT 750

WindowContext window_context;

void window_init(void)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window_context.handle = glfwCreateWindow(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, "program", NULL, NULL);
    window_context.resolution.x = DEFAULT_WINDOW_WIDTH;
    window_context.resolution.y = DEFAULT_WINDOW_HEIGHT;
    glfwGetWindowSize(window_context.handle, &window_context.width, &window_context.height);
    glfwGetWindowPos(window_context.handle, &window_context.xpos, &window_context.ypos);
    glfwGetCursorPos(window_context.handle, &window_context.cursor.x, &window_context.cursor.y);
    glfwSetWindowAspectRatio(window_context.handle, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
    
    glfwMakeContextCurrent(window_context.handle);
    glfwSetFramebufferSizeCallback(window_context.handle, window_framebuffer_size_callback);
    glfwSetMouseButtonCallback(window_context.handle, window_mouse_button_callback);
    glfwSetCursorPosCallback(window_context.handle, window_cursor_pos_callback);
    glfwSetKeyCallback(window_context.handle, window_key_callback);
    glfwSetErrorCallback(window_error_callback);
    glfwSetInputMode(window_context.handle, GLFW_LOCK_KEY_MODS, GLFW_TRUE);
    window_context.cursor.hidden = false;
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glViewport(0, 0, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
    glfwSwapInterval(0);

    window_context.dt = 0;
}

void window_update(void)
{
    glfwPollEvents();
    glfwSwapBuffers(window_context.handle);
}

void window_close(void)
{
    glfwSetWindowShouldClose(window_context.handle, 1);
}

bool window_closed(void)
{
    return glfwWindowShouldClose(window_context.handle);
}

void window_cleanup(void)
{
    glfwTerminate();
}

i32 window_width(void)
{
    return window_context.width;
}

i32 window_height(void)
{
    return window_context.height;
}
