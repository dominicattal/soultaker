#include "../game.h"
#include "../gui.h"
#include <string.h>

Client* client_create(void)
{
    Client* client = st_calloc(1, sizeof(Client));
    client->uid = game_map_uid(client, GAME_OBJ_CLIENT);
    camera_set_defaults(&client->camera);
    return client;
}

void client_update(Client* client, f32 dt)
{
    camera_update(&client->camera, dt);
}

void client_set_username(Client* client, char* username)
{
    string_free(client->username);
    client->username = username;
}

void client_destroy(Client* client)
{
    if (client->udp_address != NULL)
        socket_address_destroy(client->udp_address);
    game_free_uid(client->uid);
    string_free(client->username);
    st_free(client);
}

void client_sync_entity(Packet* packet)
{
    Client* client = game_context.this_client;
    i32 uid;
    memcpy(&uid, packet->buffer, sizeof(uid));
    log_assert(uid >= 0, "");
    log_assert(uid < MAX_UID, "");
    log_assert(game_context.uid_map_type[uid] == GAME_OBJ_ENTITY, "should be entity object to sync with");
    Entity* entity = game_context.uid_map[uid];
    client->player.entity = entity;
}

void client_change_map(void)
{
    gui_preset_load(GUI_PRESET_GAME);
    client_map_create();
}

Map* client_map_create(void)
{
    Map* map = game_context.current_map;
    if (map != NULL)
        map_destroy(map);

    map = st_calloc(1, sizeof(Map));
    map->width = MAP_MAX_WIDTH;
    map->length = MAP_MAX_LENGTH;
    map->root = NULL;
    map->roomset = NULL;
    map->tile_mask = quadmask_create(MAP_MAX_WIDTH, MAP_MAX_LENGTH);
    map->fog_mask = quadmask_create(MAP_MAX_WIDTH, MAP_MAX_LENGTH);
    map->tilemap = NULL;
    map->map_nodes = NULL;
    map->bosses = list_create();
    map->entities = list_create();
    map->tiles = list_create();
    map->walls = list_create();
    map->free_walls = list_create();
    map->projectiles = list_create();
    map->obstacles = list_create();
    map->parstacles = list_create();
    map->particles = list_create();
    map->parjicles = list_create();
    map->triggers = list_create();
    map->aoes = list_create();
    map->lines = list_create();
    map->spawn_point = vec2_create(MAP_MAX_WIDTH / 2 + 0.5, MAP_MAX_LENGTH / 2 + 0.5);
    map->active = true;
    
    game_context.current_map = map;

    return map;
}

void client_map_create_game_object(Packet* packet)
{
    Map* map = game_context.current_map;
    if (map == NULL)
        return;

    GameObj type;
    char* buffer = packet->buffer;
    memcpy(&type, buffer, sizeof(type));
    buffer += sizeof(type);

    switch (type) {
        case GAME_OBJ_ENTITY:
            Entity* entity = entity_read(buffer);
            list_append(map->entities, entity);
            game_set_uid(entity, type, entity->uid);
            break;
        case GAME_OBJ_TILE:
            Tile* tile = tile_read(buffer);
            list_append(map->tiles, tile);
            game_set_uid(tile, type, tile->uid);
            break;
        case GAME_OBJ_WALL:
            Wall* wall = wall_read(buffer);
            list_append(map->walls, wall);
            game_set_uid(wall, type, wall->uid);
            break;
        default:
            break;
    }
    log_write(DEBUG, "%d", map->entities->length);
}

