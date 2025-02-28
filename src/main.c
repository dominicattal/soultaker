#include "window.h"
#include "renderer.h"
#include "game.h"
#include "gui.h"
#include "audio.h"
#include <stdio.h>
#include <pthread.h>

void state_init(void)
{
    window_init();
    renderer_init();
    gui_init();
    game_init();
    audio_init();

}

void state_loop(void)
{
    f64 start;
    f32 dt;
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
    gui_cleanup();
    game_cleanup();
    audio_cleanup();
    renderer_cleanup();
    window_cleanup();
}

int main()
{
    #ifdef DEBUG_BUILD
    puts("DEBUG");
    #endif
    #ifdef RELEASE_BUILD
    puts("RELEASE");
    #endif
    atexit(state_cleanup);
    state_init();
    state_loop();
    return 0;
}
