#include "thread.h"
#include "log.h"
#include <pthread.h>
#include <string.h>

struct {
    const char* name;
    pthread_t thread;
} threads[NUM_THREADS];

void thread_link(const char* name)
{
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_t thread = pthread_self();
    pthread_mutex_lock(&mutex);
    for (i32 i = 0; i < NUM_THREADS; i++) {
        if (threads[i].name == NULL) {
            threads[i].name = name;
            threads[i].thread = thread;
            goto done;
        }
    }
    log_write(FATAL, "More threads found than expected");
done:
    pthread_mutex_unlock(&mutex);
}

i32 thread_get_id(const char* name)
{
    for (i32 i = 0; i < NUM_THREADS; i++)
        if (!strcmp(name, threads[i].name))
            return i;
    return -1;
}

const char* thread_get_name(i32 id)
{
    if (id < 0 || id >= NUM_THREADS || threads[id].name == NULL)
        return "Unrecognized";
    return threads[id].name;
}

i32 thread_get_self_id(void)
{
    pthread_t thread = pthread_self();
    for (i32 i = 0; i < NUM_THREADS; i++)
        if (pthread_equal(thread, threads[i].thread))
            return i;
    return -1;
}

const char* thread_get_self_name(void)
{
    pthread_t thread = pthread_self();
    for (i32 i = 0; i < NUM_THREADS; i++)
        if (pthread_equal(thread, threads[i].thread))
            return threads[i].name;
    return "Unrecognized";
}
