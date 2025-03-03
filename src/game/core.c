#include "internal.h"

GameContext game_context;

void do_something(f32 dt) 
{
    sleep(20);
}

void* game_update(void* vargp)
{
    f64 start;
    f32 dt = 0;
    entity_init();
    while (!game_context.kill_thread)
    {
        start = get_time();
        do_something(dt);
        dt = get_time() - start;
    }
    return NULL;
}

void game_init(void)
{
    game_context.player = entity_create(vec3_create(0, 0, 0));
    camera_init();
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
