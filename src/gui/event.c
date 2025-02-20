#include "internal.h"

void gui_event_queue_init(GUIEventQueue* queue)
{
    sem_init(&queue->mutex, 0, 1);
    sem_init(&queue->full, 0, EVENT_QUEUE_LENGTH);
    sem_init(&queue->empty, EVENT_QUEUE_LENGTH, EVENT_QUEUE_LENGTH);
    queue->head = queue->tail = queue->count = 0;
}

void gui_event_enqueue(GUIEventQueue* queue, GUIEvent event)
{
    sem_wait(&queue->empty);
    sem_wait(&queue->mutex);
    queue->buffer[queue->tail] = event;
    queue->tail = (queue->tail + 1) % EVENT_QUEUE_LENGTH;
    queue->count++;
    sem_post(&queue->mutex);
    sem_post(&queue->full);
}

GUIEvent gui_event_dequeue(GUIEventQueue* queue)
{
    GUIEvent event;
    if (queue->count == 0) {
        event.type = GUI_EVENT_NONE;
        return event;
    }
    sem_wait(&queue->full);
    sem_wait(&queue->mutex);
    event = queue->buffer[queue->head];
    queue->head = (queue->head + 1) % EVENT_QUEUE_LENGTH;
    queue->count--;
    sem_post(&queue->mutex);
    sem_post(&queue->empty);
    return event;
}

static void gui_event_process(GUIEvent event)
{
    switch (event.type) {
        case GUI_EVENT_CURSOR_POS_CALLBACK:
            gui_cursor_pos_callback_internal(event.args.xpos, event.args.ypos);
            break;
        default:
            break;
    }
}

void gui_event_queue_flush(GUIEventQueue* queue)
{
    GUIEvent event;
    while ((event = gui_event_dequeue(queue)).type != GUI_EVENT_NONE)
        gui_event_process(event);
}

void gui_event_queue_cleanup(GUIEventQueue* queue)
{
    sem_destroy(&queue->mutex);
    sem_destroy(&queue->full);
    sem_destroy(&queue->empty);
}
