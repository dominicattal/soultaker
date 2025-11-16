#include "internal.h"
#include "../renderer.h"
#include "../event.h"

GUIContext gui_context;

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
    gui_context.dt = 0;
    gui_comp_init();
    gui_preset_load(GUI_PRESET_TEST);
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
    gui_render_init();
    pthread_mutex_init(&gui_context.data_mutex, NULL);
    pthread_create(&gui_context.thread_id, NULL, gui_loop, NULL);
}

void gui_cleanup(void)
{
    gui_context.kill_thread = true;
    pthread_join(gui_context.thread_id, NULL);
    pthread_mutex_destroy(&gui_context.data_mutex);
    gui_render_cleanup();
}

