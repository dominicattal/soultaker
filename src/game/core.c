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
    game_context.time = 0;
    while (!game_context.kill_thread)
    {
        if (game_context.halt_game_loop) {
            sem_wait(&game_context.game_loop_sem);
            game_context.halt_game_loop = false;
            end = start = get_time();
            game_context.dt = 0;
            game_context.time = 0;
        }
        if (end - start > 0.0001) {
            game_context.dt = end - start;
            start = get_time();
            if (!game_context.paused) {
                game_context.time += game_context.dt;
                if (game_context.dt < 0.1) {
                    game_update();
                    event_queue_flush();
                    game_update_vertex_data();
                }
            }
        }
        end = get_time();
    }
    return NULL;
}

void game_halt_loop(void)
{
    sem_wait(&game_context.game_loop_sem);
    game_context.halt_game_loop = true;
}

void game_resume_loop(void)
{
    sem_post(&game_context.game_loop_sem);
}

void game_halt_input(void)
{
    game_context.halt_input = true;
}

void game_resume_input(void)
{
    game_context.halt_input = false;
}

void game_halt_render(void)
{
    game_context.halt_render = true;
}

void game_resume_render(void)
{
    game_context.halt_render = false;
}

void game_init(void)
{
    pthread_mutex_init(&game_context.getter_mutex, NULL);
    sem_init(&game_context.game_loop_sem, 0, 1);

    map_init();
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
    game_render_init();
    game_halt_loop();
    game_halt_render();
    pthread_create(&game_context.thread_id, NULL, game_loop, NULL);
}

void game_cleanup(void)
{
    game_context.kill_thread = true;
    game_context.halt_input = false;
    sem_post(&game_context.game_loop_sem);
    pthread_join(game_context.thread_id, NULL);
    pthread_mutex_destroy(&game_context.getter_mutex);
    sem_destroy(&game_context.game_loop_sem);
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
    map_cleanup();

}

f32 game_get_dt(void)
{
    return game_context.dt;
}

f32 game_get_boss_health(void)
{
    return game_context.values.boss_health;
}

f32 game_get_boss_max_health(void)
{
    return game_context.values.boss_max_health;
}

void game_summon(i32 id)
{
    vec2 position = player_position();
    entity_create(position, id);
}
