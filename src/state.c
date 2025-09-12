#include "state.h"
#include "window.h"
#include "renderer.h"
#include "game.h"
#include "gui.h"
#include "audio.h"
#include <stdio.h>
#include <pthread.h>
#include <json.h>

#ifdef DEBUG_BUILD
    #define BUILD_INFO "DEBUG"
#elif RELEASE_BUILD
    #define BUILD_INFO "RELEASE"
#endif

struct {
    HMODULE lib;
    f32 dt;
} state_context;

void state_init(void)
{
    thread_link("Main");

    state_context.lib = LoadLibrary("plugins/soultaker.dll");
    log_assert(state_context.lib, "Could not load library");

    window_init();
    renderer_init();
    gui_init();
    audio_init();
    game_init();
}

void state_loop(void)
{
    f64 start, end;
    renderer_write_texture_units();
    start = get_time();
    while (!window_closed())
    {
        window_update();
        game_process_input(state_context.dt);
        renderer_render();
        end = get_time();
        state_context.dt = end - start;
        start = end;
    }
}

void state_cleanup(void)
{
    log_unlock();
    game_cleanup();
    gui_cleanup();
    audio_cleanup();
    renderer_cleanup();
    window_cleanup();
#ifdef DEBUG_BUILD
    print_heap_info();
#endif
    log_cleanup();

    FreeLibrary(state_context.lib);
}

f32 state_get_dt(void)
{
    return state_context.dt;
}

void* state_load_function(const char* name)
{
    return GetProcAddress(state_context.lib, name);
}
