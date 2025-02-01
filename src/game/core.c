#include "../game.h"

GameContext game_context;

void* game_update(void* vargp)
{
    while (!game_context.kill_thread)
    {
    }
    return NULL;
}

void game_init(void)
{
    pthread_create(&game_context.thread_id, NULL, game_update, NULL);
}

void game_cleanup(void)
{
    game_context.kill_thread = true;
    pthread_join(game_context.thread_id, NULL);
}
