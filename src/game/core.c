#include "internal.h"
#include "../renderer.h"
#include "../event.h"

GameContext game_context;

void* game_loop(void* vargp)
{
    thread_link("Game");

    f64 start, end;
    end = start = get_time();
    game_context.dt = 0;
    log_write(INFO, "Entering game loop");
    game_context.time = 0;
    while (!game_context.kill_thread)
    {
        if (end - start > 0.0001) {
            game_context.dt = end - start;
            start = get_time();
            if (!game_context.paused) {
                game_context.time += game_context.dt;
                game_update();
                event_queue_flush();
                game_update_vertex_data();
            }
        }
        end = get_time();
    }
    log_write(INFO, "Exiting game loop");
    return NULL;
}

void game_init(void)
{
    log_write(INFO, "Initializing game...");
    game_preset_init();
    tile_init();
    wall_init();
    weapon_init();
    entity_init();
    projectile_init();
    parstacle_init();
    obstacle_init();
    particle_init();
    parjicle_init();
    camera_init();
    player_reset();
    game_render_init();

    game_preset_load(game_preset_map_id("shaitan"));

    pthread_mutex_init(&game_context.getter_mutex, NULL);
    pthread_create(&game_context.thread_id, NULL, game_loop, NULL);
    log_write(INFO, "Initialized game");
}

void game_halt_input(void)
{
    game_context.halt_input = true;
}

void game_resume_input(void)
{
    game_context.halt_input = false;
}

void game_cleanup(void)
{
    log_write(INFO, "Cleaning up game...");
    game_context.kill_thread = true;
    game_context.halt_input = false;
    pthread_join(game_context.thread_id, NULL);
    pthread_mutex_destroy(&game_context.getter_mutex);
    game_render_cleanup();
    camera_cleanup();
    weapon_cleanup();
    entity_cleanup();
    projectile_cleanup();
    tile_cleanup();
    wall_cleanup();
    parstacle_cleanup();
    obstacle_cleanup();
    particle_cleanup();
    parjicle_cleanup();
    game_preset_cleanup();

    log_write(INFO, "Cleaned up game");
}

f32 game_get_dt(void)
{
    return game_context.dt;
}

f32 game_get_boss_health(void)
{
    f32 health;
    pthread_mutex_lock(&game_context.getter_mutex);
    health = game_context.values.boss_health; 
    pthread_mutex_unlock(&game_context.getter_mutex);
    return health;
}
