#include "internal.h"

GUIContext gui_context;

static void resize_data_buffer(void)
{
    #define FLOATS_PER_COMP     13
    #define COMPS_PER_RESIZE    5
    #define FLOATS_PER_RESIZE   (FLOATS_PER_COMP * COMPS_PER_RESIZE)

    gui_context.data_swap.capacity += FLOATS_PER_RESIZE;
    if (gui_context.data_swap.buffer == NULL) {
        gui_context.data_swap.buffer = malloc(gui_context.data_swap.capacity * sizeof(GLfloat));
        assert(gui_context.data_swap.buffer != NULL);
    } else {
        GLfloat* buf = realloc(gui_context.data_swap.buffer, gui_context.data_swap.capacity * sizeof(GLfloat)); 
        assert(buf != NULL);
        gui_context.data_swap.buffer = buf;
    }

    #undef FLOATS_PER_COMP
    #undef COMPS_PER_RESIZE
    #undef FLOATS_PER_RESIZE
}

static void push_comp_data(GUIComp* comp)
{
    i32 u, v, x, y, w, h;
    u8 r, g, b, a;
    if (gui_context.data_swap.length >= gui_context.data_swap.capacity)
        resize_data_buffer();

    #define A gui_context.data_swap.buffer[gui_context.data_swap.length++]
    gui_comp_get_bbox(comp, &x, &y, &w, &h);
    A = x; A = y; A = w; A = h;
    u = 0; v = 0; w = 1; h = 1;
    A = u; A = v; A = w; A = h;
    gui_comp_get_color(comp, &r, &g, &b, &a);
    A = r; A = g; A = b; A = a;
    A = 0;
    #undef A
}

static void gui_update_helper(GUIComp* comp)
{
    push_comp_data(comp);
    for (i32 i = 0; i < gui_comp_num_children(comp); i++)
        gui_update_helper(comp->children[i]);
}

static void gui_update(void)
{
    gui_context.data_swap.length = 0;
    gui_update_helper(gui_context.root);
    GUIData tmp;
    pthread_mutex_lock(&gui_context.mutex);
    tmp = gui_context.data;
    gui_context.data = gui_context.data_swap;
    gui_context.data_swap = tmp;
    pthread_mutex_unlock(&gui_context.mutex);
}

static void* gui_loop(void* vargp)
{
    gui_comp_init();
    while (!gui_context.kill_thread)
    {
        gui_update();
        sleep(20);
    }
    gui_comp_cleanup();
    return NULL;
}

void gui_init(void)
{
    gui_render_init();
    pthread_mutex_init(&gui_context.mutex, NULL);
    pthread_create(&gui_context.thread_id, NULL, gui_loop, NULL);
}

void gui_cleanup(void)
{
    gui_context.kill_thread = true;
    pthread_join(gui_context.thread_id, NULL);
    pthread_mutex_destroy(&gui_context.mutex);
    gui_render_cleanup();
}

