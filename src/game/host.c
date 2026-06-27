#include "../game.h"
#include <string.h>

void host_handle_client_input(Packet* packet)
{
    i32 client_uid, client_controls;
    char* buffer = packet->buffer;
    memcpy(&client_uid, buffer, sizeof(client_uid));
    buffer += sizeof(client_uid);
    memcpy(&client_controls, buffer, sizeof(client_controls));
    buffer += sizeof(client_controls);
    Client* client = game_context.uid_map[client_uid];
    client->control_flags = client_controls;
    memcpy(&client->camera, buffer, sizeof(client->camera));
    buffer += sizeof(client->camera);

    //log_write(DEBUG, "%f %f", client->camera.window_cursor_position.x, client->camera.window_cursor_position.y)
}

void host_swap_items(Packet* packet)
{
    i32 client_uid, slot1_idx, slot2_idx;
    memcpy(&client_uid, packet->buffer, sizeof(i32));
    memcpy(&slot1_idx, packet->buffer + sizeof(i32), sizeof(i32));
    memcpy(&slot2_idx, packet->buffer + 2*sizeof(i32), sizeof(i32));

    log_write(DEBUG, "%d %d %d", client_uid, slot1_idx, slot2_idx);

    Client* client = game_context.uid_map[client_uid];
    Item** slot1 = &client->player.inventory.items[slot1_idx];
    Item** slot2 = &client->player.inventory.items[slot2_idx];
    inventory_swap_items_for_client(client, slot1, slot2);
    inventory_sync(client);
}

void host_create_game_obj(i32 uid)
{
    Packet packet;
    GameObj type;
    static char packet_buffer[UDP_MAX_PAYLOAD];
    packet.buffer = packet_buffer + PACKET_HEADER_BYTES;

    packet.id = PACKET_CREATE_GAME_OBJ;
    type = game_context.uid_map_type[uid];

    memcpy(packet.buffer, &type, sizeof(type));
    size_t size = game_object_write(type, game_context.uid_map[uid], packet.buffer + sizeof(type));

    packet.length = size + sizeof(type);

    memcpy(packet_buffer, &packet.length, sizeof(packet.length));
    memcpy(packet_buffer + sizeof(packet.length), &packet.id, sizeof(packet.id));

    game_net_send_tcp_packet_to_clients(&packet);
    
}

void host_destroy_game_obj(i32 uid)
{
    Packet packet;
    GameObj type;
    static char packet_buffer[UDP_MAX_PAYLOAD];

    packet.buffer = packet_buffer + PACKET_HEADER_BYTES;
    packet.id = PACKET_DESTROY_GAME_OBJ;

    memcpy(packet.buffer, &uid, sizeof(uid));

    packet.length = sizeof(uid) + sizeof(type);

    memcpy(packet_buffer, &packet.length, sizeof(packet.length));
    memcpy(packet_buffer + sizeof(packet.length), &packet.id, sizeof(packet.id));

    game_net_send_tcp_packet_to_clients(&packet);
}
