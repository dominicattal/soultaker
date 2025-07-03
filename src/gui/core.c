#include "internal.h"
#include "../renderer.h"
#include "../event.h"

GUIContext gui_context;

void align_comp_position_x(i32* position_x, u8 halign, i32 size_x, i32 x, i32 w)
{
    if      (halign == ALIGN_LEFT)       *position_x += x;
    else if (halign == ALIGN_CENTER_POS) *position_x += (size_x - w) / 2 + x;
    else if (halign == ALIGN_CENTER_NEG) *position_x += (size_x - w) / 2 - x;
    else if (halign == ALIGN_RIGHT)      *position_x += size_x - w - x;
}

void align_comp_position_y(i32* position_y, u8 valign, i32 size_y, i32 y, i32 h)
{
    if      (valign == ALIGN_TOP)        *position_y += size_y - h - y;
    else if (valign == ALIGN_CENTER_POS) *position_y += (size_y - h) / 2 + y;
    else if (valign == ALIGN_CENTER_NEG) *position_y += (size_y - h) / 2 - y;
    else if (valign == ALIGN_BOTTOM)     *position_y += y;
}

static void gui_update_comps_helper(GUIComp* comp, f32 dt)
{
    gui_comp_update(comp, dt);
    for (i32 i = 0; i < gui_comp_num_children(comp); i++)
        gui_update_comps_helper(comp->children[i], dt);
}

static void gui_update_comps(f32 dt)
{
    gui_update_comps_helper(gui_context.root, dt);
}

static void* gui_loop(void* vargp)
{
    thread_link("GUI");

    f64 start, end;
    end = start = get_time();
    log_write(INFO, "Entering gui loop");
    gui_context.dt = 0;
    gui_comp_init();
    gui_preset_load(GUI_PRESET_DEBUG);
    while (!gui_context.kill_thread)
    {
        if (end - start > 0.0001) {
            gui_context.dt = end - start;
            start = get_time();
            gui_update_comps(gui_context.dt);
            gui_update_vertex_data();
            event_queue_flush();
        }
        end = get_time();
    }
    log_write(INFO, "Exiting gui loop");
    gui_comp_cleanup();
    return NULL;
}

f32 gui_get_dt(void)
{
    return gui_context.dt;
}

GUIComp* gui_get_event_comp(GUIEventCompEnum type)
{
    return gui_context.event_comps[type];
}

void gui_set_event_comp(GUIEventCompEnum type, GUIComp* comp)
{
    gui_context.event_comps[type] = comp;
}

bool gui_event_comp_equal(GUIEventCompEnum type, GUIComp* comp)
{
    return gui_context.event_comps[type] == comp;
}

void gui_init(void)
{
    log_write(INFO, "Initializing GUI...");
    gui_render_init();
    pthread_mutex_init(&gui_context.data_mutex, NULL);
    pthread_create(&gui_context.thread_id, NULL, gui_loop, NULL);
    log_write(INFO, "Iniitalized GUI");
}

void gui_cleanup(void)
{
    log_write(INFO, "Cleaning up GUI...");
    gui_context.kill_thread = true;
    pthread_join(gui_context.thread_id, NULL);
    pthread_mutex_destroy(&gui_context.data_mutex);
    gui_render_cleanup();
    log_write(INFO, "Cleaned up GUI");
}

