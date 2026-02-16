#include "../window.h"
#include "../renderer.h"

#define DEFAULT_WINDOW_WIDTH 1000
#define DEFAULT_WINDOW_HEIGHT 750

WindowContext window_context;

static void init_controls(void)
{
    for (i32 i = 0; i < NUM_CONTROLS; i++) {
        window_context.controls[i].ctrl_enum = i;
        window_context.controls[i].glfw_enum = GLFW_ENUM_UNBOUND;
        window_context.controls[i].mods = 0;
        window_context.control_idx_map[i] = i;
    }

    window_set_control_binding(CTRL_INTERACT, GLFW_KEY_F, 0);
    window_set_control_binding(CTRL_TEST1, GLFW_KEY_F, 0);
    window_set_control_binding(CTRL_TEST2, GLFW_KEY_0, 0);
    window_set_control_binding(CTRL_TEST3, GLFW_MOUSE_BUTTON_LEFT, 0);
    window_set_control_binding(CTRL_TEST4, GLFW_MOUSE_BUTTON_RIGHT, 0);
    window_set_control_binding(CTRL_TEST5, GLFW_MOUSE_BUTTON_5, 0);
    window_set_control_binding(CTRL_TEST6, GLFW_MOUSE_BUTTON_5, 1);
}

void window_init(void)
{
    glfwSetErrorCallback(window_error_callback);
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window_context.handle = glfwCreateWindow(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, "soultaker", NULL, NULL);
    window_context.resolution.x = DEFAULT_WINDOW_WIDTH;
    window_context.resolution.y = DEFAULT_WINDOW_HEIGHT;
    glfwGetWindowSize(window_context.handle, &window_context.width, &window_context.height);
    //glfwGetWindowPos(window_context.handle, &window_context.xpos, &window_context.ypos);
    window_context.xpos = DEFAULT_WINDOW_WIDTH;
    window_context.ypos = DEFAULT_WINDOW_HEIGHT;
    glfwGetCursorPos(window_context.handle, &window_context.cursor.x, &window_context.cursor.y);
    glfwSetWindowAspectRatio(window_context.handle, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
    
    glfwMakeContextCurrent(window_context.handle);
    glfwSetFramebufferSizeCallback(window_context.handle, window_framebuffer_size_callback);
    glfwSetMouseButtonCallback(window_context.handle, window_mouse_button_callback);
    glfwSetCursorPosCallback(window_context.handle, window_cursor_pos_callback);
    glfwSetKeyCallback(window_context.handle, window_key_callback);
    glfwSetCharCallback(window_context.handle, window_char_callback);
    glfwSetInputMode(window_context.handle, GLFW_LOCK_KEY_MODS, GLFW_TRUE);
    window_context.cursor.hidden = false;
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glViewport(0, 0, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
    glfwSwapInterval(1);
    
    window_context.dt = 0;

    GLfloat aspect_ratio = (GLfloat)window_context.width / window_context.height;

    glGenBuffers(1, &window_context.ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, window_context.ubo);
    glBindBufferBase(GL_UNIFORM_BUFFER, UBO_INDEX_WINDOW, window_context.ubo);
    glBufferData(GL_UNIFORM_BUFFER, 3 * sizeof(GLint), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GLint), &window_context.width);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(GLint), sizeof(GLint), &window_context.height);
    glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(GLint), sizeof(GLfloat), &aspect_ratio);

    init_controls();
}

void window_update(void)
{
}

void window_close(void)
{
    glfwSetWindowShouldClose(window_context.handle, 1);
}

void window_toggle_fullscreen(void)
{
    glfwMaximizeWindow(window_context.handle);
}

bool window_closed(void)
{
    return glfwWindowShouldClose(window_context.handle);
}

void window_cleanup(void)
{
    if (window_context.ubo != 0)
        glDeleteBuffers(1, &window_context.ubo);
    glfwTerminate();
}

bool window_get_key(i32 key)
{
    return glfwGetKey(window_context.handle, key);
}

bool window_get_mouse_button(i32 button)
{
    return glfwGetMouseButton(window_context.handle, button);
}

i32 window_resolution_x(void)
{
    return window_context.resolution.x;
}

i32 window_resolution_y(void)
{
    return window_context.resolution.y;
}

f32 window_aspect_ratio(void)
{
    return (f32)window_context.width / window_context.height;
}

i32 window_width(void)
{
    return window_context.width;
}

i32 window_height(void)
{
    return window_context.height;
}

f64 window_cursor_position_x(void)
{
    return window_context.cursor.x;
}

f64 window_cursor_position_y(void)
{
    return window_context.cursor.y;
}

vec2 window_cursor_position(void)
{
    return vec2_create(window_context.cursor.x, window_context.cursor.y);
}
