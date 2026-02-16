#include "state.h"
#include "event.h"
#include "window.h"
#include "renderer.h"
#include "game.h"
#include "gui.h"
#include "audio.h"
#include <stdio.h>
#include <pthread.h>
#include <math.h>

StateContext state_context;

void state_init(void)
{
    pthread_mutex_init(&state_context.mutex, 0);

    log_init();
    state_context.config = config_create();

    thread_link("Main");

    event_init();
    window_init();
    renderer_init();
    gui_init();
    game_init();
}

void state_loop(void)
{
    f64 start, end;
    while (!window_closed()) {
        start = get_time();
        glfwPollEvents();
        game_process_input();
        renderer_render();
        glfwSwapBuffers(window_context.handle);
        end = get_time();
        state_context.dt = end - start;
        start = end;
    }
}

f32 state_dt(void)
{
    return state_context.dt;
}

void state_cleanup(void)
{
    // add signals so game doesnt cleanup b4 gui finishes with it
    log_unlock();
    game_cleanup();
    gui_cleanup();
    renderer_cleanup();
    window_cleanup();
    event_cleanup();

    config_destroy(state_context.config);

#ifdef DEBUG_BUILD
    print_heap_info();
#endif

    log_cleanup();

    pthread_mutex_destroy(&state_context.mutex);
}

void* state_load_function(const char* name)
{
    return config_get_function(state_context.config, name);
    //return dlsym(state_context.handle, name);
}
