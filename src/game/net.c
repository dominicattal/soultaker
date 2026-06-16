#include "../game.h"
#include <string.h>
#include <pthread.h>

extern GameContext game_context;

static bool kill_net_host_thread;
static bool kill_net_handler_threads;

void game_net_set_host_ip(const char* ip)
{
    if (game_context.host_ip != NULL)
        string_free(game_context.host_ip);
    game_context.host_ip = string_copy(ip);
}

void game_net_set_host_tcp_port(const char* port)
{
    if (game_context.host_tcp_port != NULL)
        string_free(game_context.host_tcp_port);
    game_context.host_tcp_port = string_copy(port);
}

void game_net_set_host_udp_port(const char* port)
{
    if (game_context.host_udp_port != NULL)
        string_free(game_context.host_udp_port);
    game_context.host_udp_port = string_copy(port);
}

static void host_handle_tcp_packet(Client* client, Packet* packet)
{
    switch (packet->id) {
        case PACKET_MESSAGE:
            log_write(DEBUG, "TCP message: %s", packet->buffer);
            break;
        default:
            break;
    }
}

static void* host_tcp_client_handler(void* vargp)
{
    Packet* packet;
    Client* client = vargp;
    while (!kill_net_host_thread) {
        packet = socket_recv(client->tcp_socket);
        if (packet == NULL) {
            log_write(WARNING, "packet is null");
            continue;
        }
        host_handle_tcp_packet(client, packet);
        packet_destroy(packet);
    }
    return NULL;
}

static void host_handle_udp_packet(Packet* packet)
{
    switch (packet->id) {
        case PACKET_MESSAGE:
            log_write(DEBUG, "UDP message: %s", packet->buffer);
            break;
        default:
            break;
    }
}

static void* host_udp_handler(void* vargp)
{
    pthread_barrier_t* net_listen_barrier = vargp;
    NetContext* net_ctx = game_context.net;
    char* ip = game_context.host_ip;
    Socket* listen_socket = socket_create(net_ctx, ip, NULL, BIT_UDP);
    SocketAddr* addr = NULL;
    Packet* packet;
    socket_bind(listen_socket);
    game_context.this_client->udp_socket = listen_socket;
    game_net_set_host_udp_port(socket_port(listen_socket));
    log_write(DEBUG, "Listening over UDP on %s:%s", socket_ip(listen_socket), socket_port(listen_socket));
    pthread_barrier_wait(net_listen_barrier);
    while (!kill_net_host_thread) {
        packet = socket_recvfrom(listen_socket, &addr);
        if (packet == NULL) {
            log_write(WARNING, "packet is null");
            continue;
        }
        host_handle_udp_packet(packet);
        packet_destroy(packet);
        socket_address_destroy(addr);
    }
    return NULL;
}

static void test_connectivity(Client* client)
{
    Packet* packet;
    char* buf = "Hello from TCP";
    packet = packet_create(PACKET_MESSAGE, strlen(buf)+1, buf);
    game_net_send_packet_tcp(client, packet);
    packet_destroy(packet);

    buf = "Hello from UDP";
    packet = packet_create(PACKET_MESSAGE, strlen(buf)+1, buf);
    game_net_send_packet_udp(client, packet);
    packet_destroy(packet);
}

static void* host_tcp_handler(void* vargp)
{
    pthread_barrier_t* net_listen_barrier = vargp;
    NetContext* net_ctx = game_context.net;
    Client* this_client = game_context.this_client;
    char* ip = game_context.host_ip;
    char* port = game_context.host_tcp_port;
    Socket* listen_socket;
    Socket* client_socket;
    Packet* packet;
    char uint_buf[32];
    pthread_barrier_wait(net_listen_barrier);
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

        packet = packet_create(PACKET_HOST_UDP_PORT, strlen(game_context.host_udp_port)+1, game_context.host_udp_port);
        socket_send(client_socket, packet);
        packet_destroy(packet);

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
        log_assert(packet->id == PACKET_CLIENT_UDP_PORT, "");
        client->udp_address = socket_address_create(client_ip, packet->buffer);
        packet_destroy(packet);

        packet = socket_recv(client_socket);
        log_assert(packet->id == PACKET_CLIENT_TO_HOST_USERNAME, "");
        client->username = string_copy(packet->buffer);
        packet_destroy(packet);

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, host_tcp_client_handler, client);

        test_connectivity(client);
    }
    networking_join_sockets(game_context.net);
    return NULL;
fail:
    game_net_stop_hosting();
    return NULL;
}

static void client_handle_tcp_packet(Packet* packet)
{
    switch (packet->id) {
        case PACKET_MESSAGE:
            log_write(DEBUG, "TCP message: %s", packet->buffer);
            break;
        case PACKET_LOAD_GAME:
            log_write(DEBUG, "Loading game");
            client_change_map();
            break;
        case PACKET_CREATE_GAME_OBJ:
            client_map_create_game_object(packet);
            break;
        case PACKET_UPDATE_GAME_OBJ:
            client_map_update_game_object(packet);
            break;
        case PACKET_DESTROY_GAME_OBJ:
            client_map_destroy_game_object(packet);
            break;
        case PACKET_SYNC_CLIENT_ENTITY:
            client_sync_entity(packet);
            break;
        case PACKET_CREATE_MAP_NODES:
            client_map_create_map_nodes(packet);
            break;
        case PACKET_CLEAR_FOG:
            client_map_clear_fog(packet);
            break;
        default:
            log_write(WARNING, "Received unknown packed: %d", packet->id);
    }
}

static void* client_tcp_handler(void* vargp)
{
    Socket* server_socket = vargp;
    Packet* packet;
    log_write(DEBUG, "initalized client tcp handler");
    while (!kill_net_handler_threads) {
        packet = socket_recv(server_socket);
        if (packet == NULL)
            continue;
        pthread_mutex_lock(&game_context.handler_thread_mutex);
        client_handle_tcp_packet(packet);
        pthread_mutex_unlock(&game_context.handler_thread_mutex);
        packet_destroy(packet);
    }
    return NULL;
}

static void client_handle_udp_packet(Packet* packet)
{
    switch (packet->id) {
        case PACKET_MESSAGE:
            log_write(DEBUG, "UDP message: %s", packet->buffer);
            break;
        case PACKET_UPDATE_GAME_OBJ:
            client_map_update_game_object(packet);
            break;
        default:
            break;
    }
}

static void* client_udp_handler(void* vargp)
{
    Packet* packet;
    SocketAddr* server_addr;
    Socket* udp_socket = vargp;
    log_write(DEBUG, "initalized client udp handler");
    while (!kill_net_handler_threads) {
        packet = socket_recvfrom(udp_socket, &server_addr);
        if (packet == NULL) {
            log_write(WARNING, "received null packet");
            continue;
        }
        pthread_mutex_lock(&game_context.handler_thread_mutex);
        client_handle_udp_packet(packet);
        pthread_mutex_unlock(&game_context.handler_thread_mutex);
        packet_destroy(packet);
        socket_address_destroy(server_addr);
    }
    return NULL;
}

void game_net_join(const char* ip, const char* port)
{
    if (game_context.net != NULL) {
        log_write(WARNING, "game already joined, ignoring");
        return;
    }

    game_context.net = networking_init();

    Packet* packet;
    Socket* server_socket = socket_create(game_context.net, ip, port, BIT_TCP);
    if (!socket_connect(server_socket)) {
        log_write(WARNING, "could not connect to server");
        networking_cleanup(game_context.net);
        game_context.net = NULL;
        return;
    }

    game_context.singleplayer = false;

    game_net_set_host_ip(ip);
    game_net_set_host_tcp_port(port);

    Client* this_client = game_context.this_client;
    game_context.host_client = client_create();
    game_context.host_client->tcp_socket = server_socket;
    list_append(game_context.clients, game_context.host_client);

    packet = socket_recv(server_socket);
    log_assert(packet->id == PACKET_HOST_UDP_PORT, "");
    game_net_set_host_udp_port(packet->buffer);
    log_write(DEBUG, "udp port: %s", packet->buffer);
    packet_destroy(packet);

    this_client->udp_socket = socket_create(game_context.net, "127.0.0.1", NULL, BIT_UDP);
    socket_bind(this_client->udp_socket);
    log_write(DEBUG, "listening on udp %s:%s", socket_ip(this_client->udp_socket), socket_port(this_client->udp_socket));
    game_context.host_client->udp_address = socket_address_create(ip, game_context.host_udp_port);

    packet = socket_recv(server_socket);
    log_assert(packet->id == PACKET_HOST_TO_CLIENT_USERNAME, "");
    game_context.host_client->username = string_copy(packet->buffer);
    packet_destroy(packet);

    packet = socket_recv(server_socket);
    log_assert(packet->id == PACKET_HOST_TO_CLIENT_HOST_UID, "");
    game_context.host_client->uid = atoi(packet->buffer);
    game_set_uid(game_context.host_client, GAME_OBJ_CLIENT, game_context.host_client->uid);
    packet_destroy(packet);

    packet = socket_recv(server_socket);
    log_assert(packet->id == PACKET_HOST_TO_CLIENT_CLIENT_UID, "");
    this_client->uid = atoi(packet->buffer);
    game_set_uid(this_client, GAME_OBJ_CLIENT, this_client->uid);
    packet_destroy(packet);

    packet = packet_create(PACKET_CLIENT_UDP_PORT, strlen(socket_port(this_client->udp_socket))+1, socket_port(this_client->udp_socket));
    socket_send(server_socket, packet);
    packet_destroy(packet);

    packet = packet_create(PACKET_CLIENT_TO_HOST_USERNAME, strlen(this_client->username)+1, this_client->username);
    socket_send(server_socket, packet);
    packet_destroy(packet);

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, client_tcp_handler, server_socket);
    pthread_create(&thread_id, NULL, client_udp_handler, this_client->udp_socket);
    socket_set_thread_id(server_socket, thread_id);

    test_connectivity(game_context.host_client);
}

void game_net_start_hosting(const char* ip, const char* port)
{
    if (game_context.net != NULL) {
        log_write(WARNING, "game is already hosting, ignoring");
        return;
    }

    game_context.net = networking_init();
    game_net_set_host_ip(ip);
    game_net_set_host_tcp_port(port);
    game_context.hosting = true;
    game_context.singleplayer = false;
    kill_net_host_thread = false;
    kill_net_handler_threads = false;

    pthread_barrier_t net_listen_barrier;
    pthread_barrier_init(&net_listen_barrier, NULL, 3);
    pthread_create(&game_context.net_tcp_listen_thread_id, NULL, host_tcp_handler, &net_listen_barrier);
    pthread_create(&game_context.net_udp_listen_thread_id, NULL, host_udp_handler, &net_listen_barrier);
    pthread_barrier_wait(&net_listen_barrier);
    pthread_barrier_destroy(&net_listen_barrier);

    log_write(DEBUG, "hosting");
}

void game_net_stop_hosting(void)
{
    if (!game_context.hosting) {
        log_write(WARNING, "game is not hosting");
        return;
    }

    kill_net_host_thread = true;
    kill_net_handler_threads = true;
    game_context.hosting = false;
    game_context.singleplayer = true;
    networking_shutdown_sockets(game_context.net);
    pthread_join(game_context.net_tcp_listen_thread_id, NULL);
    pthread_join(game_context.net_udp_listen_thread_id, NULL);
    networking_cleanup(game_context.net);
    game_context.net = NULL;

    string_free(game_context.host_ip);
    string_free(game_context.host_tcp_port);
    string_free(game_context.host_udp_port);
}

void game_net_cleanup(void)
{
    if (game_context.hosting)
        game_net_stop_hosting();
    else if (game_context.net != NULL) {
        kill_net_handler_threads = true;
        networking_cleanup(game_context.net);
        game_context.hosting = false;
        game_context.singleplayer = true;
        game_context.net = NULL;
        string_free(game_context.host_ip);
        string_free(game_context.host_tcp_port);
        string_free(game_context.host_udp_port);
    }
}

void game_net_send_tcp_packet_to_clients(Packet* packet)
{
    if (!game_context.hosting)
        return;
    for (i32 i = 0; i < game_context.clients->length; i++) {
        Client* client = list_get(game_context.clients, i);
        if (client != game_context.this_client)
            socket_send(client->tcp_socket, packet);
    }
}

void game_net_send_udp_packet_to_clients(Packet* packet)
{
    if (!game_context.hosting)
        return;
    for (i32 i = 0; i < game_context.clients->length; i++) {
        Client* client = list_get(game_context.clients, i);
        if (client != game_context.this_client)
            socket_sendto(game_context.this_client->udp_socket, client->udp_address, packet);
    }
}

void game_net_send_packet_udp(Client* client, Packet* packet)
{
    socket_sendto(game_context.this_client->udp_socket, client->udp_address, packet);
}

void game_net_send_packet_tcp(Client* client, Packet* packet)
{
    socket_send(client->tcp_socket, packet);
}
