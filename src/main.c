#include "util.h"
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

static f32 dt;

void state_init(void)
{
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
        game_process_input(dt);
        renderer_render();
        dt = get_time() - start;
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
}

f32 state_dt(void)
{
    return dt;
}

int main(int argc, char** argv)
{
    atexit(state_cleanup);
    log_init(argv[0]);
    state_init();
    state_loop();
    return 0;
}
