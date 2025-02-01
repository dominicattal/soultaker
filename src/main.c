#include "window.h"
#include "renderer.h"
#include "game.h"
#include "gui.h"
#include "audio.h"
#include <stdio.h>

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
    while (!window_closed())
    {
        window_update();
        renderer_render();
    }
}

void state_cleanup(void)
{
    window_cleanup();
    renderer_cleanup();
    gui_cleanup();
    game_cleanup();
    audio_cleanup();
}

int main()
{
    atexit(state_cleanup);
    state_init();
    state_loop();
    return 0;
}
