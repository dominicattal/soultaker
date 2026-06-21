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
    Player* player = &client->player;
    if (!game_context.hosting && !game_context.singleplayer) {
        Packet packet;
        static char buffer[256];
        char* ptr = buffer;
        packet.buffer = buffer + PACKET_HEADER_BYTES;
        packet.length = sizeof(client->uid) 
                      + sizeof(client->control_flags)
                      + sizeof(client->camera.facing)
                      + sizeof(client->camera.right)
                      + sizeof(client->camera.follow);
        packet.id = PACKET_CLIENT_INPUT;
        memcpy(ptr, &packet.length, sizeof(packet.length));
        ptr += sizeof(packet.length);
        memcpy(ptr, &packet.id, sizeof(packet.id));
        ptr += sizeof(packet.id);
        memcpy(ptr, &client->uid, sizeof(client->uid));
        ptr += sizeof(client->uid);
        memcpy(ptr, &client->control_flags, sizeof(client->control_flags));
        ptr += sizeof(client->control_flags);
        memcpy(ptr, &client->camera.facing, sizeof(client->camera.facing));
        ptr += sizeof(client->camera.facing);
        memcpy(ptr, &client->camera.right, sizeof(client->camera.right));
        ptr += sizeof(client->camera.right);
        memcpy(ptr, &client->camera.follow, sizeof(client->camera.follow));
        ptr += sizeof(client->camera.follow);
        game_net_send_packet_udp(game_context.host_client, &packet);
    }

    camera_update(&client->camera, dt);
    player_update(&client->player, dt);
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
    log_write(DEBUG, "AAAA");
    //log_assert(game_context.uid_map_type[uid] == GAME_OBJ_ENTITY, "should be entity object to sync with");
    client->player.entity_uid = uid;
    client->player.synced = false;
    //Entity* entity = game_context.uid_map[uid];
    //client->player.entity = entity;
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
    map->map_nodes = st_calloc(map->width * map->length, sizeof(MapNode*));
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
            Entity* entity = st_calloc(1, sizeof(Entity));
            entity_read(entity, buffer);
            list_append(map->entities, entity);
            game_set_uid(entity, type, entity->uid);
            break;
        case GAME_OBJ_PROJECTILE:
            Projectile* proj = st_calloc(1, sizeof(Projectile));
            projectile_read(proj, buffer);
            list_append(map->projectiles, proj);
            game_set_uid(proj, type, proj->uid);
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
        case GAME_OBJ_OBSTACLE:
            Obstacle* obstacle = obstacle_read(buffer);
            list_append(map->obstacles, obstacle);
            game_set_uid(obstacle, type, obstacle->uid);
            break;
        case GAME_OBJ_PARSTACLE:
            Parstacle* parstacle = parstacle_read(buffer);
            list_append(map->parstacles, parstacle);
            game_set_uid(parstacle, type, parstacle->uid);
            break;
        case GAME_OBJ_ITEM:
            Item* item = st_calloc(1, sizeof(Item));
            item_read(item, buffer);
            game_set_uid(item, type, item->uid);
            break;
        default:
            break;
    }
}

void client_map_update_game_object(Packet* packet)
{
    Map* map = game_context.current_map;
    if (map == NULL)
        return;

    GameObj type;
    i32 high;
    char* buffer = packet->buffer;
    size_t size;
    memcpy(&type, buffer, sizeof(type));
    buffer += sizeof(type);
    memcpy(&high, buffer, sizeof(type));
    buffer += sizeof(high);

    switch (type) {
        case GAME_OBJ_ENTITY:
            Entity entity;
            size = entity_sizeof();
            for (i32 i = 0; i < high; i++) {
                entity_read(&entity, buffer);
                map_queue_entity(entity);
                buffer += size;
            }
            break;
        case GAME_OBJ_PROJECTILE:
            Projectile proj;
            size = projectile_sizeof();
            for (i32 i = 0; i < high; i++) {
                projectile_read(&proj, buffer);
                map_queue_projectile(proj);
                buffer += size;
            }
            break;
        default:
            break;
    }
}

void client_map_destroy_game_object(Packet* packet)
{
    Map* map = game_context.current_map;
    if (map == NULL)
        return;

    i32 uid;
    memcpy(&uid, packet->buffer, sizeof(uid));

    GameObj type = game_context.uid_map_type[uid];

    switch (type) {
        case GAME_OBJ_PROJECTILE:
            Projectile* this_proj = game_context.uid_map[uid];
            if (this_proj != NULL) {
                this_proj->lifetime = -1;
            }
            break;
        default:
            break;
    }

    game_free_uid(uid);
}

void client_map_clear_fog(Packet* packet)
{
    Map* map = game_context.current_map;
    size_t size = sizeof(i32);
    void* node;
    i32 x1, x2, z1, z2;
    memcpy((char*)&x1, packet->buffer, size);
    memcpy((char*)&x2, packet->buffer + size, size);
    memcpy((char*)&z1, packet->buffer + 2 * size, size);
    memcpy((char*)&z2, packet->buffer + 3 * size, size);
    memcpy(&node, packet->buffer + 4 * size, sizeof(void*));
    for (i32 z = z1; z <= z2; z++)
        for (i32 x = x1; x <= x2; x++)
            if (node == map->map_nodes[z * map->width + x])
                quadmask_set(map->fog_mask, x, z);

    game_render_update_obstacles();
    game_render_update_parstacles();
    game_render_update_tiles();
    game_render_update_walls();
}

void client_map_create_map_nodes(Packet* packet)
{
    Map* map = game_context.current_map;
    if  (map == NULL)
        return;

    memcpy(map->map_nodes, packet->buffer, packet->length);
}

void client_map_create_particle(Packet* packet)
{
    Particle particle;
    memcpy(&particle, packet->buffer, sizeof(particle));
    map_queue_particle(particle);
}

void client_map_create_parjicle(Packet* packet)
{
    Parjicle parjicle;
    memcpy(&parjicle, packet->buffer, sizeof(parjicle));
    map_queue_parjicle(parjicle);
}

void client_update_stats(Packet* packet)
{
    Player* player = &game_context.this_client->player;
    size_t size = NUM_STATS * sizeof(f32);
    memcpy(player->base_stats, packet->buffer, size);
    memcpy(player->stats, packet->buffer + size, size);
}
