#include "../game.h"
#include <string.h>

extern GameContext game_context;

static bool kill_net_host_thread;

static void* net_host_udp_listener(void* vargp)
{
    NetContext* net_ctx = game_context.net;
    char* ip = game_context.ip;
    char* port = game_context.port;
    Socket* listen_socket;
    while (!kill_net_host_thread) {
    }
    return NULL;
}

static void* net_host_tcp_listener(void* vargp)
{
    NetContext* net_ctx = game_context.net;
    Client* this_client = game_context.this_client;
    char* ip = game_context.ip;
    char* port = game_context.port;
    Socket* listen_socket;
    Socket* client_socket;
    Packet* packet;
    char uint_buf[32];
    while (!kill_net_host_thread) {
        listen_socket = socket_create(net_ctx, ip, port, BIT_TCP);
        if (listen_socket == NULL) {
            log_write(CRITICAL, "failed to create tcp listen socket");
            goto fail;
        }
        if (!socket_bind(listen_socket)) {
            log_write(CRITICAL, "failed to bind tcp listen socket");
            goto fail;
        }
        if (!socket_listen(listen_socket)) {
            log_write(CRITICAL, "failed to listen for tcp listen socket");
            goto fail;
        }
        client_socket = socket_accept(listen_socket);
        socket_destroy(listen_socket);
        if (client_socket == NULL) {
            if (!kill_net_host_thread) {
                log_write(CRITICAL, "client socket is NULL for tcp listen socket");
                goto fail;
            }
            break;
        }

        const char* client_ip = socket_ip(client_socket);
        const char* client_port = socket_port(client_socket);
        log_write(DEBUG, "connected to %s:%s", client_ip, client_port);

        Client* client = client_create();
        list_append(game_context.clients, client);
        client->tcp_socket = client_socket;

        packet = packet_create(PACKET_HOST_TO_CLIENT_USERNAME, strlen(this_client->username)+1, this_client->username);
        socket_send(client_socket, packet);
        packet_destroy(packet);

        sprintf(uint_buf, "%u", this_client->uid);
        packet = packet_create(PACKET_HOST_TO_CLIENT_HOST_UID, sizeof(this_client->uid)+1, uint_buf);
        socket_send(client_socket, packet);
        packet_destroy(packet);

        sprintf(uint_buf, "%u", client->uid);
        packet = packet_create(PACKET_HOST_TO_CLIENT_CLIENT_UID, sizeof(client->uid)+1, uint_buf);
        socket_send(client_socket, packet);
        packet_destroy(packet);

        packet = socket_recv(client_socket);
        log_assert(packet->id == PACKET_CLIENT_TO_HOST_USERNAME, "");
        client->username = string_copy(packet->buffer);
        packet_destroy(packet);
    }
    networking_join_sockets(game_context.net);
    return NULL;
fail:
    game_net_stop_hosting();
    return NULL;
}

void game_net_join(char* ip, char* port)
{
    if (game_context.net != NULL) {
        log_write(WARNING, "game already joined, ignoring");
        return;
    }

    game_context.net = networking_init();

    Socket* server_socket = socket_create(game_context.net, ip, port, BIT_TCP);
    if (!socket_connect(server_socket)) {
        log_write(WARNING, "could not connect to server");
        networking_cleanup(game_context.net);
        game_context.net = NULL;
        return;
    }

    game_context.ip = ip;
    game_context.port = port;

    Packet* packet;
    Client* this_client = game_context.this_client;

    game_context.host_client = client_create();
    list_append(game_context.clients, game_context.host_client);
    packet = socket_recv(server_socket);
    log_assert(packet->id == PACKET_HOST_TO_CLIENT_USERNAME, "");
    game_context.host_client->username = string_copy(packet->buffer);
    packet_destroy(packet);
    packet = socket_recv(server_socket);
    log_assert(packet->id == PACKET_HOST_TO_CLIENT_HOST_UID, "");
    game_context.host_client->uid = atoi(packet->buffer);
    packet_destroy(packet);
    packet = socket_recv(server_socket);
    log_assert(packet->id == PACKET_HOST_TO_CLIENT_CLIENT_UID, "");
    this_client->uid = atoi(packet->buffer);
    packet_destroy(packet);

    packet = packet_create(PACKET_CLIENT_TO_HOST_USERNAME, strlen(this_client->username)+1, this_client->username);
    socket_send(server_socket, packet);
    packet_destroy(packet);
}

void game_net_start_hosting(char* ip, char* port)
{
    if (game_context.net != NULL) {
        log_write(WARNING, "game is already hosting, ignoring");
        return;
    }

    game_context.net = networking_init();
    game_context.ip = ip;
    game_context.port = port;
    game_context.hosting = true;
    kill_net_host_thread = false;
    pthread_create(&game_context.net_tcp_listen_thread_id, NULL, net_host_tcp_listener, NULL);
    pthread_create(&game_context.net_udp_listen_thread_id, NULL, net_host_udp_listener, NULL);

    log_write(DEBUG, "hosting");
}

void game_net_stop_hosting(void)
{
    if (!game_context.hosting) {
        log_write(WARNING, "game is not hosting");
        return;
    }

    kill_net_host_thread = true;
    game_context.hosting = false;
    networking_shutdown_sockets(game_context.net);
    pthread_join(game_context.net_tcp_listen_thread_id, NULL);
    pthread_join(game_context.net_udp_listen_thread_id, NULL);
    networking_cleanup(game_context.net);
    game_context.net = NULL;
}

void game_net_cleanup(void)
{
    if (game_context.hosting)
        game_net_stop_hosting();
    if (game_context.net != NULL) {
        networking_cleanup(game_context.net);
        game_context.hosting = false;
        game_context.net = NULL;
    }
}

