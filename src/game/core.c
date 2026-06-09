#include "../game.h"
#include "../renderer.h"
#include "../event.h"
#include <string.h>

GameContext game_context;

#define MIN_DT  0.0001

void* game_loop(void* vargp)
{
    Client* client;
    f64 start, end;
    pthread_mutex_t* init_mutex = vargp;
    thread_link("Game");
    end = start = get_time();
    game_context.dt = 0;
    game_context.time = 0;
    game_context.clients = list_create();
    client = client_create();
    client_set_username(client, string_copy("fancy"));
    game_context.this_client = client;
    list_append(game_context.clients, client);
    gui_comp_init();
    pthread_mutex_unlock(init_mutex);
    gui_preset_load(GUI_PRESET_MP);
    //gui_preset_load(GUI_PRESET_GAME);
    game_resume_render();
    //map_create(map_get_id("outpost1"));
    game_context.singleplayer = true;
    while (!game_context.kill_thread)
    {
        while (end - start < MIN_DT)
            end = get_time();
        game_context.dt = end - start;
        if (game_context.dt > 0.1)
            game_context.dt = 0.1;
        game_context.time += game_context.dt;
        start = end;
        pthread_mutex_lock(&game_context.handler_thread_mutex);
        event_queue_flush();
        gui_update_comps(game_context.dt);
        if (game_context.current_map != NULL) {
            if (!game_context.paused)
                map_update(game_context.current_map);
            camera_update();
            game_update_vertex_data();
        }
        pthread_mutex_unlock(&game_context.handler_thread_mutex);
    }
    gui_comp_cleanup();
    for (i32 i = 0; i < game_context.clients->length; i++) {
        client = list_get(game_context.clients, i);
        client_destroy(client);
    }
    list_destroy(game_context.clients);
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
}

static void write_map_data_and_send(Map* map)
{
    char* mut_buffer;
    char* org_buffer;
    size_t buffer_len = 0;
    i32 tile_length = map->tiles->length;
    i32 wall_length = map->walls->length;
    buffer_len += sizeof(i32);
    buffer_len += tile_length * tile_sizeof();
    buffer_len += sizeof(i32);
    buffer_len += wall_length * wall_sizeof();
    mut_buffer = org_buffer = st_malloc(buffer_len);
    memcpy(mut_buffer, &tile_length, sizeof(i32));
    mut_buffer += sizeof(i32);
    for (i32 i = 0; i < tile_length; i++)
        mut_buffer = tile_write(list_get(map->tiles, i), mut_buffer);
    memcpy(mut_buffer, &wall_length, sizeof(i32));
    mut_buffer += sizeof(i32);
    for (i32 i = 0; i < wall_length; i++)
        mut_buffer = wall_write(list_get(map->walls, i), mut_buffer);

    log_write(DEBUG, "%p %p", mut_buffer, org_buffer + buffer_len);
    Packet* packet = packet_create(PACKET_LOAD_GAME, buffer_len, org_buffer);
    socket_send_all(game_context.net, packet);
    packet_destroy(packet);
    st_free(org_buffer);
}

void game_change_map(i32 id)
{
    gui_preset_load(GUI_PRESET_GAME);
    Map* map = map_create(id);
    if (game_context.hosting)
        write_map_data_and_send(map);
}

void game_change_map_from_binary(i32 buffer_len, char* buffer)
{
    gui_preset_load(GUI_PRESET_GAME);
    map_create_from_binary(buffer_len, buffer);
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
    game_net_cleanup();
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
