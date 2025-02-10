#include "internal.h"
#include "../renderer.h"

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

static void push_comp_data(GUIComp* comp, i32 x, i32 y)
{
    i32 w, h;
    f32 u1, v1, u2, v2;
    u8 r, g, b, a;
    u32 loc;
    if (gui_context.data_swap.length >= gui_context.data_swap.capacity)
        resize_data_buffer();

    #define A gui_context.data_swap.buffer[gui_context.data_swap.length++]
    gui_comp_get_size(comp, &w, &h);
    A = x; A = y; A = w; A = h;
    gui_comp_get_color(comp, &r, &g, &b, &a);
    A = r; A = g; A = b; A = a;
    texture_info(gui_comp_tex(comp), &u1, &v1, &u2, &v2, &loc);
    A = u1; A = v1; A = u2; A = v2;
    A = loc;
    #undef A

    gui_context.data_swap.instance_count++;
}

static void gui_update_helper(GUIComp* comp, i32 position_x, i32 position_y, i32 size_x, i32 size_y)
{
    i32 x, y, w, h;
    u8 halign, valign;
    gui_comp_get_bbox(comp, &x, &y, &w, &h);
    gui_comp_get_align(comp, &halign, &valign);
    
    if      (halign == ALIGN_LEFT)       position_x += x;
    else if (halign == ALIGN_CENTER_POS) position_x += (size_x - w) / 2 + x;
    else if (halign == ALIGN_CENTER_NEG) position_x += (size_x - w) / 2 - x;
    else if (halign == ALIGN_RIGHT)      position_x += size_x - w - x;

    if      (valign == ALIGN_TOP)        position_y += y;
    else if (valign == ALIGN_CENTER_POS) position_y += (size_y - h) / 2 + y;
    else if (valign == ALIGN_CENTER_NEG) position_y += (size_y - h) / 2 - y;
    else if (valign == ALIGN_BOTTOM)     position_y += size_y - h - y;

    push_comp_data(comp, position_x, position_y);
    for (i32 i = 0; i < gui_comp_num_children(comp); i++)
        gui_update_helper(comp->children[i], position_x, position_y, w, h);
}

static void gui_update(void)
{
    gui_context.data_swap.instance_count = 0;
    gui_context.data_swap.length = 0;
    gui_update_helper(gui_context.root, 0, 0, 0, 0);
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

