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

GlobalContext global_context;

void state_init(void)
{
    global_context.lib = LoadLibrary("plugins/soultaker.dll");
    log_assert(global_context.lib, "Could not load library");

    window_init();
    renderer_init();
    gui_init();
    audio_init();
    game_init();
}

void state_loop(void)
{
    f64 start;
    while (!window_closed())
    {
        start = get_time();
        window_update();
        game_process_input(global_context.dt);
        renderer_render();
        global_context.dt = get_time() - start;
    }
}

void state_cleanup(void)
{
    log_unlock();
    game_cleanup();
    audio_cleanup();
    gui_cleanup();
    renderer_cleanup();
    window_cleanup();
    log_cleanup();

    FreeLibrary(global_context.lib);
}
