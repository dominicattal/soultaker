#ifdef __WIN32

#include "net.h"
#include "log.h"
#include "malloc.h"
#include "extra.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

typedef struct SocketAddr {
    struct addrinfo addr;
} SocketAddr;

typedef struct Socket {
    NetContext* ctx;
    struct addrinfo* info;
    struct Socket* prev;
    struct Socket* next;
    char* ip;
    char* port;
    SOCKET* sock;
    pthread_t thread_id;
    bool connected;
    bool has_thread;
} Socket;

typedef struct NetContext {
    WSADATA wsa_data;
    Socket* head;
    Socket* tail;
    pthread_mutex_t mutex;
    bool active;
} NetContext;

WSADATA wsa_data;
bool startup = false;

static WSADATA wsa_startup_if_not(void)
{
    if (!startup) {
        if (WSAStartup(MAKEWORD(2,2), &wsa_data))
            printf("WSAStartup failed: %d\n", WSAGetLastError());
        else
            startup = true;
    }
    return wsa_data;
}

NetContext* networking_init(void)
{
    NetContext* ctx = st_malloc(sizeof(NetContext));
    ctx->wsa_data = wsa_startup_if_not();
    if (!startup) {
        st_free(ctx);
        return NULL;
    }
    ctx->head = ctx->tail = NULL;
    pthread_mutex_init(&ctx->mutex, NULL);
    ctx->active = true;
    return ctx;
}

void networking_shutdown_sockets(NetContext* ctx)
{
    Socket* sock;
    pthread_mutex_lock(&ctx->mutex);
    sock = ctx->head;
    while (sock != NULL) {
        if (sock->sock != NULL) {
            closesocket(*sock->sock);
            st_free(sock->sock);
            sock->sock = NULL;
        }
        sock = sock->next;
    }
    ctx->active = false;
    pthread_mutex_unlock(&ctx->mutex);
}

void networking_join_sockets(NetContext* ctx)
{
    Socket* sock;
    pthread_mutex_lock(&ctx->mutex);
    sock = ctx->head;
    while (sock != NULL) {
        if (sock->has_thread)
            pthread_kill(sock->thread_id, 0);
        sock = sock->next;
    }
    pthread_mutex_unlock(&ctx->mutex);
}


void networking_cleanup(NetContext* ctx)
{
    Socket* sock = ctx->head;
    Socket* next;
    while (sock != NULL) {
        next = sock->next;
        socket_destroy(sock);
        sock = next;
    }
    pthread_mutex_destroy(&ctx->mutex);
    WSACleanup();
    startup = false;
    st_free(ctx);
}

int networking_get_last_error(void)
{
    return WSAGetLastError();
}

static Socket* get_free_socket(NetContext* ctx)
{
    Socket* sock = NULL;
    pthread_mutex_lock(&ctx->mutex);
    if (!ctx->active) goto unlock;
    sock = st_calloc(1, sizeof(Socket));
    sock->ctx = ctx;
    sock->connected = false;
    sock->next = NULL;
    sock->prev = ctx->tail;
    sock->sock = NULL;
    sock->info = NULL;
    if (ctx->head == NULL) {
        ctx->head = sock;
    } else {
        ctx->tail->next = sock;
    }
    ctx->tail = sock;
unlock:
    pthread_mutex_unlock(&ctx->mutex);
    return sock;
}

Socket* socket_create(NetContext* ctx, const char* ip, const char* port, int flags)
{
    struct addrinfo* result = NULL;
    struct addrinfo hints;
    SOCKET* new_socket = NULL;
    Socket* res_socket = NULL;

    res_socket = get_free_socket(ctx);
    if (res_socket == NULL)
        return NULL;

    int tcp = flags & 1;
    int will_bind = (flags>>1) & 1;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = (tcp) ? SOCK_STREAM : SOCK_DGRAM;
    hints.ai_protocol = (tcp) ? IPPROTO_TCP : IPPROTO_UDP;
    hints.ai_flags = (will_bind) ? AI_PASSIVE : 0;
    if (getaddrinfo(ip, port, &hints, &result))
        goto fail;
    new_socket = st_malloc(sizeof(SOCKET));
    *new_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (*new_socket == INVALID_SOCKET)
        goto fail_free_addr_info;

    //BOOL opt_val = TRUE;
    //int opt_len = sizeof(BOOL);
    //setsockopt(*new_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt_val, opt_len);

    //int iopt_val = 5000;
    //opt_len = sizeof(int);
    //setsockopt(*new_socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&iopt_val, opt_len);
    //setsockopt(*new_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&iopt_val, opt_len);

    res_socket->sock = new_socket;
    res_socket->info = result;

    return res_socket;

fail_free_addr_info:
    freeaddrinfo(result);
fail:
    return NULL;
}

SocketAddr* socket_get_address(Socket* socket)
{
    return (SocketAddr*)socket->info;
}

SocketAddr* socket_address_create(const char* ip, const char* port)
{
    struct addrinfo hints = {0};
    struct addrinfo* sock_addr = NULL;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    int rc = getaddrinfo(ip, port, &hints, &sock_addr);
    if (rc != 0)
        log_write(FATAL, "getaddrinfo failed %s %s (%d): %s\n", ip, port, rc, gai_strerror(rc));
    return (SocketAddr*)sock_addr;
}

void socket_address_destroy(SocketAddr* addr)
{
    freeaddrinfo(&addr->addr); 
}

static void socket_set_ip_and_port(Socket* sock)
{
    if (sock->ip != NULL)
        st_free(sock->ip);
    sock->ip = st_malloc(NI_MAXHOST * sizeof(char));
    if (sock->port != NULL)
        st_free(sock->port);
    sock->port = st_malloc(NI_MAXSERV * sizeof(char));
    struct sockaddr_storage addr;
    socklen_t len = sizeof(addr);
    getsockname(*sock->sock, (struct sockaddr*)&addr, &len);
    getnameinfo((struct sockaddr*)&addr,
                len,
                sock->ip,
                NI_MAXHOST * sizeof(char),
                sock->port,
                NI_MAXSERV * sizeof(char),
                NI_NUMERICHOST | NI_NUMERICSERV);
}

bool socket_bind(Socket* sock)
{
    bool result = bind(*sock->sock, sock->info->ai_addr, (int)sock->info->ai_addrlen) != SOCKET_ERROR;
    socket_set_ip_and_port(sock);
    return result;
}

bool socket_listen(Socket* sock)
{
    return listen(*sock->sock, SOMAXCONN) != SOCKET_ERROR;
}

Socket* socket_accept(Socket* sock)
{
    Socket* client_socket;
    SOCKET* new_socket;

    client_socket = get_free_socket(sock->ctx);
    if (client_socket == NULL)
        return NULL;
    new_socket = st_malloc(sizeof(SOCKET));
    *new_socket = accept(*sock->sock, NULL, NULL);
    if (*new_socket == INVALID_SOCKET) {
        st_free(new_socket);
        return NULL;
    }
    client_socket->sock = new_socket;
    client_socket->connected = true;
    socket_set_ip_and_port(client_socket);
    return client_socket;
}

bool socket_connect(Socket* sock)
{
    struct addrinfo* ptr;
    for (ptr = sock->info; ptr != NULL; ptr = ptr->ai_next) {
        *sock->sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (*sock->sock == INVALID_SOCKET)
            return false;
        if (connect(*sock->sock, ptr->ai_addr, (int)ptr->ai_addrlen) != SOCKET_ERROR) {
            sock->connected = true;
            socket_set_ip_and_port(sock);
            return true;
        }
    }
    return false;
}

void socket_destroy(Socket* sock)
{
    NetContext* ctx;
    ctx = sock->ctx;
    pthread_mutex_lock(&sock->ctx->mutex);
    if (sock->sock != NULL) {
        shutdown(*sock->sock, SD_BOTH);
        closesocket(*sock->sock);
    }
    if (sock->info != NULL)
        freeaddrinfo(sock->info);
    sock->connected = false;
    if (sock->sock != NULL) {
        st_free(sock->sock);
        sock->sock = NULL;
    }
    sock->info = NULL;
    if (sock == ctx->head)
        ctx->head = sock->next;
    else
        sock->prev->next = sock->next;
    if (sock == ctx->tail)
        ctx->tail = sock->prev;
    else
        sock->next->prev = sock->prev;
    if (sock->ip != NULL)
        st_free(sock->ip);
    if (sock->port != NULL)
        st_free(sock->port);
    st_free(sock);
    pthread_mutex_unlock(&ctx->mutex);
}

bool socket_send(Socket* sock, Packet* packet)
{
    return send(*sock->sock, packet->buffer - PACKET_HEADER_BYTES, packet->length + PACKET_HEADER_BYTES, 0) != SOCKET_ERROR;
}

void socket_send_all(NetContext* ctx, Packet* packet)
{
    Socket* cur;
    pthread_mutex_lock(&ctx->mutex);
    cur = ctx->head;
    while (cur != NULL) {
        if (cur->connected)
            socket_send(cur, packet);
        cur = cur->next;
    }
    pthread_mutex_unlock(&ctx->mutex);
}

bool socket_sendto(Socket* src_socket, SocketAddr* dst_addr, Packet* packet)
{
    return sendto(*src_socket->sock, 
                  packet->buffer - PACKET_HEADER_BYTES, 
                  packet->length + PACKET_HEADER_BYTES, 
                  0, 
                  dst_addr->addr.ai_addr,
                  (int)dst_addr->addr.ai_addrlen) != SOCKET_ERROR;
}

Packet* socket_recv(Socket* sock)
{
    Packet* packet;
    ssize_t length;
    ssize_t received = 0;
    char buffer[PACKET_HEADER_BYTES];
    while (received < PACKET_HEADER_BYTES) {
        length = recv(*sock->sock, buffer + received, PACKET_HEADER_BYTES - received, 0);
        if (length == SOCKET_ERROR) {
            log_write(CRITICAL, "recvfailed: WsaGetLastError() = %d", WSAGetLastError());
            return NULL;
        }
        received += length;
    }

    packet = st_malloc(sizeof(Packet));
    packet->id = ((u8)buffer[4]<<8)|(u8)buffer[5];
    packet->length = ((u8)buffer[0]<<24)|((u8)buffer[1]<<16)|((u8)buffer[2]<<8)|(u8)buffer[3];
    packet->buffer = st_malloc((packet->length + PACKET_HEADER_BYTES) * sizeof(char));
    memcpy(packet->buffer, buffer, PACKET_HEADER_BYTES);

    received = 0;
    while (received < (ssize_t)packet->length) {
        length = recv(*sock->sock, packet->buffer + PACKET_HEADER_BYTES - received, packet->length - received, 0);
        if (length == SOCKET_ERROR) {
            log_write(CRITICAL, "recvfailed: WsaGetLastError() = %d", WSAGetLastError());
            st_free(packet->buffer);
            st_free(packet);
            return NULL;
        }
        received += length;
    }
    packet->buffer += PACKET_HEADER_BYTES;
    return packet;
}

Packet* socket_recvfrom(Socket* src_socket, SocketAddr** dst_addr)
{
    Packet* packet;
    socklen_t client_len = sizeof(struct sockaddr);
    char buffer[UDP_MAX_PAYLOAD];
    *dst_addr = st_malloc(sizeof(SocketAddr));
    ssize_t len = recvfrom(*src_socket->sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&(*dst_addr)->addr, &client_len);
    if (len == SOCKET_ERROR) {
        log_write(CRITICAL, "recvfrom failed: WsaGetLastError() = %d", WSAGetLastError());
        return NULL;
    }
    packet = st_malloc(sizeof(Packet));
    packet->id = ((u8)buffer[4]<<8)|(u8)buffer[5];
    packet->length = ((u8)buffer[0]<<24)|((u8)buffer[1]<<16)|((u8)buffer[2]<<8)|(u8)buffer[3];
    packet->buffer = st_malloc((packet->length + PACKET_HEADER_BYTES) * sizeof(char));
    memcpy(packet->buffer, buffer, packet->length + PACKET_HEADER_BYTES);
    packet->buffer += PACKET_HEADER_BYTES;
    return packet;
}

bool socket_connected(Socket* sock)
{
    return sock->connected;
}

void socket_set_thread_id(Socket* sock, pthread_t thread_id)
{
    sock->has_thread = true;
    sock->thread_id = thread_id;
}

const char* socket_ip(Socket* sock)
{
    return sock->ip;
}

const char* socket_port(Socket* sock)
{
    return sock->port;
}

#endif
