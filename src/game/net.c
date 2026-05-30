#include "../game.h"
#include <string.h>

extern GameContext game_context;

static bool kill_net_host_thread;

static void* net_host_listener(void* vargp)
{
    NetContext* net_ctx = game_context.net;
    char* ip = game_context.ip;
    char* port = game_context.port;
    Socket* listen_socket;
    Socket* client_socket;
    Packet* packet;
    while (!kill_net_host_thread) {
        listen_socket = socket_create(net_ctx, ip, port, BIT_TCP);
        if (listen_socket == NULL) {
            log_write(CRITICAL, "failed to create listen socket");
            goto fail;
        }
        if (!socket_bind(listen_socket)) {
            log_write(CRITICAL, "failed to bind listen socket");
            goto fail;
        }
        if (!socket_listen(listen_socket)) {
            log_write(CRITICAL, "failed to listen for listen socket");
            goto fail;
        }
        client_socket = socket_accept(listen_socket);
        socket_destroy(listen_socket);
        if (client_socket == NULL) {
            if (!kill_net_host_thread) {
                log_write(CRITICAL, "client socket is NULL");
                goto fail;
            }
            break;
        }

        const char* client_ip = socket_ip(client_socket);
        const char* client_port = socket_port(client_socket);
        log_write(DEBUG, "connected to %s:%s", client_ip, client_port);

        packet = socket_recv(client_socket);
        log_write(DEBUG, packet->buffer);
        packet_destroy(packet);

        char* msg = "hello from server";
        packet = packet_create(PACKET_TEST, strlen(msg)+1, msg);
        socket_send(client_socket, packet);
        packet_destroy(packet);
    }
    networking_join_sockets(game_context.net);
    return NULL;
fail:
    game_net_stop_hosting();
    return NULL;
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
    game_context.clients = list_create();
    kill_net_host_thread = false;
    pthread_create(&game_context.net_thread_id, NULL, net_host_listener, NULL);

    log_write(DEBUG, "hosting");
}

void game_net_stop_hosting(void)
{
    if (game_context.net == NULL) {
        log_write(WARNING, "game is not hosting");
        return;
    }

    kill_net_host_thread = true;
    networking_shutdown_sockets(game_context.net);
    pthread_join(game_context.net_thread_id, NULL);
    networking_cleanup(game_context.net);
    if (game_context.clients != NULL)
        list_destroy(game_context.clients);
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
    game_context.clients = list_create();

    log_write(DEBUG, "connected");
    char* msg = "hello from client";
    Packet* packet = packet_create(PACKET_TEST, strlen(msg)+1, msg);
    socket_send(server_socket, packet);
    packet_destroy(packet);

    packet = socket_recv(server_socket);
    log_write(DEBUG, packet->buffer);
    packet_destroy(packet);
}
