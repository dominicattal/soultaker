#include "internal.h"
#include "../renderer.h"

GameApi game_api = {
    .texture_get_id = texture_get_id,
    .entity_get_direction = entity_get_direction
};

GameContext game_context;

void* game_loop(void* vargp)
{
    f64 start;
    log_write(INFO, "Entering game loop");
    while (!game_context.kill_thread)
    {
        start = get_time();
        game_update();
        game_update_vertex_data();
        game_context.dt = get_time() - start;
    }
    log_write(INFO, "Exiting game loop");
    return NULL;
}

void game_init(void)
{
    log_write(INFO, "Initializing game...");
    tile_init();
    wall_init();
    entity_init();
    projectile_init();
    parstacle_init();
    obstacle_init();
    particle_init();
    parjicle_init();
    camera_init();
    game_render_init();
    game_context.data.update_tile_buffer = true;
    game_context.data.update_wall_buffer = true;
    game_context.data.update_parstacle_buffer = true;
    game_context.data.update_obstacle_buffer = true;
    game_context.data_swap.update_tile_buffer = true;
    game_context.data_swap.update_wall_buffer = true;
    game_context.data_swap.update_parstacle_buffer = true;
    game_context.data_swap.update_obstacle_buffer = true;
    pthread_mutex_init(&game_context.data_mutex, NULL);
    pthread_create(&game_context.thread_id, NULL, game_loop, NULL);
    log_write(INFO, "Initialized game");
}

void game_cleanup(void)
{
    log_write(INFO, "Cleaning up game...");
    game_context.kill_thread = true;
    pthread_join(game_context.thread_id, NULL);
    pthread_mutex_destroy(&game_context.data_mutex);
    game_render_cleanup();
    camera_cleanup();
    entity_cleanup();
    projectile_cleanup();
    tile_cleanup();
    wall_cleanup();
    parstacle_cleanup();
    obstacle_cleanup();
    particle_cleanup();
    parjicle_cleanup();

    free(game_context.data.projectile_buffer);
    free(game_context.data.entity_buffer);
    free(game_context.data.tile_buffer);
    free(game_context.data.wall_buffer);
    free(game_context.data.parstacle_buffer);
    free(game_context.data.obstacle_buffer);
    free(game_context.data.particle_buffer);
    free(game_context.data.parjicle_buffer);
    free(game_context.data_swap.projectile_buffer);
    free(game_context.data_swap.entity_buffer);
    free(game_context.data_swap.tile_buffer);
    free(game_context.data_swap.wall_buffer);
    free(game_context.data_swap.parstacle_buffer);
    free(game_context.data_swap.obstacle_buffer);
    free(game_context.data_swap.particle_buffer);
    free(game_context.data_swap.parjicle_buffer);
    log_write(INFO, "Cleaned up game");
}

f32 game_dt(void)
{
    return game_context.dt;
}
