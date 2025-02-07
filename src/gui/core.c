#include "internal.h"

GUIContext gui_context;

void* gui_loop(void* vargp)
{
    gui_comp_init();
    while (!gui_context.kill_thread)
    {
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

