#include "../game.h"
#include "../renderer.h"
#include "../event.h"
#include "../gui/../game.h"

GameContext game_context;

#define MIN_DT  0.0001
//#define MIN_DT  0.005

void* game_loop(void* vargp)
{
    f64 start, end;
    pthread_mutex_t* init_mutex = vargp;
    thread_link("Game");
    end = start = get_time();
    game_context.dt = 0;
    game_context.time = 0;
    gui_comp_init();
    pthread_mutex_unlock(init_mutex);
    gui_preset_load(GUI_PRESET_GAME);
    game_resume_render();
    map_create(map_get_id("outpost1"));
    while (!game_context.kill_thread)
    {
        while (end - start < MIN_DT)
            end = get_time();
        game_context.dt = end - start;
        if (game_context.dt > 0.1)
            game_context.dt = 0.1;
        game_context.time += game_context.dt;
        start = end;
        gui_update_comps(game_context.dt);
        event_queue_flush();
        if (game_context.current_map != NULL) {
            if (!game_context.paused)
                map_update(game_context.current_map);
            game_update_vertex_data();
        }
    }
    gui_comp_cleanup();
    return NULL;
}

void game_change_map(i32 id)
{
    log_assert(game_context.current_map != NULL, "should not have happened");
    gui_preset_load(GUI_PRESET_GAME);
    map_create(id);
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

void game_pause(void)
{
    game_context.paused = true;
}
 
void game_resume(void)
{
    game_context.paused = false;
}

void game_init(void)
{
    pthread_mutex_t init_mutex;
    pthread_mutex_init(&init_mutex, NULL);

    map_init();
    item_init();
    entity_init();
    synergy_init();
    camera_init();
    game_halt_render();
    game_render_init();
    gui_render_init();
    pthread_mutex_lock(&init_mutex);
    pthread_create(&game_context.thread_id, NULL, game_loop, &init_mutex);
    pthread_mutex_lock(&init_mutex);
    pthread_mutex_destroy(&init_mutex);
}

void game_cleanup(void)
{
    game_context.kill_thread = true;
    game_context.halt_input = false;
    pthread_join(game_context.thread_id, NULL);
    pthread_mutex_destroy(&game_context.getter_mutex);
    player_cleanup(&game_context.player);
    game_render_cleanup();
    gui_render_cleanup();
    camera_cleanup();
    map_cleanup();
    item_cleanup();
    synergy_cleanup();
    entity_cleanup();
}

f32 game_get_dt(void)
{
    return game_context.dt;
}

void game_summon(i32 id)
{
    //vec2 position = player_position();
    //entity_create(position, id);
}
