#include "../gui.h"
#include "../renderer.h"
#include "../event.h"

GUIContext gui_context;

GUIComp* gui_get_event_comp(GUIEventCompEnum type)
{
    return gui_context.event_comps[type];
}

void gui_set_event_comp(GUIEventCompEnum type, GUIComp* comp)
{
    GUIComp* prev_comp = gui_context.event_comps[type];
    gui_context.event_comps[type] = comp;
    if (prev_comp != NULL) {
        prev_comp->event_id = GUI_COMP_DEFAULT;
    }
    if (comp != NULL) {
        log_assert(comp->event_id == GUI_COMP_DEFAULT, "cannot assign event comp to another event comp");
        comp->event_id = type;
    }
}

bool gui_event_comp_equal(GUIEventCompEnum type, GUIComp* comp)
{
    return gui_context.event_comps[type] == comp;
}

void gui_init(void)
{
    //gui_render_init();
    //pthread_mutex_init(&gui_context.data_mutex, NULL);
    //pthread_create(&gui_context.thread_id, NULL, gui_loop, NULL);
}

void gui_cleanup(void)
{
    //gui_context.kill_thread = true;
    //pthread_join(gui_context.thread_id, NULL);
    //pthread_mutex_destroy(&gui_context.data_mutex);
    //gui_render_cleanup();
}

