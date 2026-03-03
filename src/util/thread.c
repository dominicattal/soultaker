#include "thread.h"
#include "log.h"
#include <pthread.h>
#include <string.h>

struct {
    const char* name;
    pthread_t thread;
} threads[NUM_THREADS];

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void thread_link(const char* name)
{
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
    i32 res = -1;
    pthread_mutex_lock(&mutex);
    for (i32 i = 0; i < NUM_THREADS; i++) {
        if (!strcmp(name, threads[i].name)) {
            res = i;
            break;
        }
    }
    pthread_mutex_unlock(&mutex);
    return res;
}

const char* thread_get_name(i32 id)
{
    if (id < 0 || id >= NUM_THREADS || threads[id].name == NULL)
        return "N/A";
    return threads[id].name;
}

i32 thread_get_self_id(void)
{
    pthread_t thread = pthread_self();
    i32 res = -1;
    pthread_mutex_lock(&mutex);
    for (i32 i = 0; i < NUM_THREADS; i++) {
        if (pthread_equal(thread, threads[i].thread)) {
            res = i;
            break;
        }
    }
    pthread_mutex_unlock(&mutex);
    return res;
}

const char* thread_get_self_name(void)
{
    pthread_t thread = pthread_self();
    const char* res = "N/A";
    pthread_mutex_lock(&mutex);
    for (i32 i = 0; i < NUM_THREADS; i++) {
        if (pthread_equal(thread, threads[i].thread)) {
            res = threads[i].name;
            break;
        }
    }
    pthread_mutex_unlock(&mutex);
    return res;
}
