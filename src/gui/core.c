#include "../gui.h"

GUIContext gui_context;

void* gui_update(void* vargp)
{
    while (!gui_context.kill_thread)
    {
    }
    return NULL;
}

void gui_init(void)
{
    pthread_create(&gui_context.thread_id, NULL, gui_update, NULL);
}

void gui_cleanup(void)
{
    gui_context.kill_thread = true;
    pthread_join(gui_context.thread_id, NULL);
}

