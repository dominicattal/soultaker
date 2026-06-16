#include "../game.h"
#include "../renderer.h"
#include "../event.h"
#include <string.h>

GameContext game_context;

void handle_callback(void)
{
    if (game_context.cursor_moved)
        gui_cursor_pos_callback(window_context.cursor.x, window_context.cursor.y);
}

void* game_loop(void* vargp)
{
    Client* client;
    f64 start, end;
    f64 real_start;
    f32 dt;
    pthread_mutex_t* init_mutex = vargp;
    thread_link("Game");
    end = start = get_time();
    game_context.time = 0;
    game_context.tps = 144;
    game_context.timestep = 1.0 / game_context.tps;
    game_context.clients = list_create();
    game_context.created_uids = list_i32_create();
    game_context.freed_uids = list_i32_create();
    client = client_create();
    client_set_username(client, string_copy("fancy"));
    game_context.this_client = client;
    list_append(game_context.clients, client);

    map_init();
    item_init();
    entity_init();
    synergy_init();
    gui_comp_init();

    pthread_mutex_unlock(init_mutex);
    gui_preset_load(GUI_PRESET_MP);
    //gui_preset_load(GUI_PRESET_GAME);
    game_resume_render();
    //map_create(map_get_id("outpost1"));
    game_context.singleplayer = true;
    while (!game_context.kill_thread)
    {
        while (end - start < game_context.timestep)
            end = get_time();
        dt = game_context.timestep;
        game_context.time += dt;
        start = end;
        real_start = get_time();
        pthread_mutex_lock(&game_context.handler_thread_mutex);
        handle_callback();
        event_queue_flush();
        gui_update_comps(dt);
        game_process_input(dt);
        if (game_context.current_map != NULL) {
            if (!game_context.paused)
                map_update(game_context.current_map, dt);
            client_update(game_context.this_client, dt);
            game_update_vertex_data();
        }
        pthread_mutex_unlock(&game_context.handler_thread_mutex);
        game_context.real_dt = get_time() - real_start;
    }
    gui_comp_cleanup();
    map_cleanup();
    item_cleanup();
    synergy_cleanup();
    entity_cleanup();
    game_net_cleanup();
    for (i32 i = 0; i < game_context.clients->length; i++) {
        client = list_get(game_context.clients, i);
        client_destroy(client);
    }
    list_destroy(game_context.clients);
    list_i32_destroy(game_context.created_uids);
    list_i32_destroy(game_context.freed_uids);
    game_context.this_client = NULL;
    return NULL;
}

i32 game_map_uid(void* obj, GameObj type)
{
    i32 uid, cnt;
    cnt = 0;
    while (game_context.uid_map[game_context.uid_cursor] != NULL && cnt < MAX_UID) {
        game_context.uid_cursor = (game_context.uid_cursor+1) % MAX_UID;
        cnt++;
    }
    uid = game_context.uid_cursor;
    if (cnt == MAX_UID) 
        return -1;
    game_context.uid_map[uid] = obj;
    game_context.uid_map_type[uid] = type;

    if (game_context.hosting)
        list_i32_append(game_context.created_uids, uid);

    return uid;
}

void game_set_uid(void* obj, GameObj type, i32 uid)
{
    game_context.uid_map[uid] = obj;
    game_context.uid_map_type[uid] = type;
}

void game_free_uid(i32 uid)
{
    game_context.uid_map[uid] = NULL;
    game_context.uid_map_type[uid] = GAME_OBJ_NONE;

    if (game_context.hosting)
        list_i32_append(game_context.freed_uids, uid);
}

void game_change_map(i32 id)
{
    if (game_context.hosting) {
        Packet* packet = packet_create(PACKET_LOAD_GAME, 0, NULL);
        game_net_send_tcp_packet_to_clients(packet);
        packet_destroy(packet);
    }

    gui_preset_load(GUI_PRESET_GAME);
    Map* map = map_create(id);
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
    game_render_cleanup();
    gui_render_cleanup();
}

void game_summon(i32 id)
{
    //vec2 position = player_position();
    //entity_create(position, id);
}

size_t game_object_write(GameObj type, void* obj, char* buffer)
{
    switch (type) {
        case GAME_OBJ_ENTITY:
            entity_write(obj, buffer);
            return entity_sizeof();
        case GAME_OBJ_PROJECTILE:
            projectile_write(obj, buffer);
            return entity_sizeof();
        case GAME_OBJ_TILE:
            tile_write(obj, buffer);
            return entity_sizeof();
        case GAME_OBJ_WALL:
            wall_write(obj, buffer);
            return wall_sizeof();
        default:
            break;
    }
    log_write(WARNING, "writing unrecognized object %d", type);
    return 0;
}
