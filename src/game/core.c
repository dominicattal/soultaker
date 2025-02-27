#include "internal.h"

GameContext game_context;

void* game_update(void* vargp)
{
    camera_init();
    while (!game_context.kill_thread)
    {
        game_process_input();
        sleep(20);
    }
    return NULL;
}

void game_init(void)
{
    game_render_init();
    pthread_create(&game_context.thread_id, NULL, game_update, NULL);
}

void game_cleanup(void)
{
    game_context.kill_thread = true;
    pthread_join(game_context.thread_id, NULL);
    game_render_cleanup();
    camera_cleanup();
}
