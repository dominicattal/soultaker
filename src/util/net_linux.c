#ifdef __linux__

#include "net.h"
#include "malloc.h"
#include "extra.h"
#include "log.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>

typedef struct SocketAddr {
    struct sockaddr_in addr;
} SocketAddr;

typedef struct Socket {
    NetContext* ctx;
    struct Socket* prev;
    struct Socket* next;
    struct sockaddr_in addr;
    pthread_t thread_id;
    char* ip;
    char* port;
    int fd;
    bool has_thread;
    bool connected;
    bool tcp;
} Socket;

typedef struct NetContext {
   pthread_mutex_t mutex;
   Socket* head;
   Socket* tail;
   bool active;
} NetContext;

NetContext* networking_init(void)
{
    NetContext* ctx = st_malloc(sizeof(NetContext));
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
        if (sock->fd != -1)
            shutdown(sock->fd, SHUT_RDWR);
        sock->fd = -1;
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
        sock->fd = -1;
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
    st_free(ctx);
}

static Socket* get_free_socket(NetContext* ctx)
{
    Socket* sock = NULL;
    pthread_mutex_lock(&ctx->mutex);
    if (!ctx->active) goto unlock;
    sock = st_malloc(sizeof(Socket));
    sock->ctx = ctx;
    sock->connected = false;
    sock->next = NULL;
    sock->prev = ctx->tail;
    sock->has_thread = false;
    sock->ip = NULL;
    sock->port = NULL;
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

Socket* socket_create(NetContext* ctx, const char* ip, const char* port_str, int flags)
{
    Socket* sock;
    int port;

    if (ip == NULL)
        return NULL;

    port = (port_str == NULL) ? 0 : atoi(port_str);
    sock = get_free_socket(ctx);
    if (sock == NULL)
        goto fail;

    if (flags & BIT_TCP) {
        sock->fd = socket(AF_INET, SOCK_STREAM, 0);
        sock->tcp = true;
    } else {
        sock->fd = socket(AF_INET, SOCK_DGRAM, 0);
        sock->tcp = false;
    }
    sock->addr.sin_family = AF_INET;
    sock->addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &sock->addr.sin_addr);
    setsockopt(sock->fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    setsockopt(sock->fd, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));

    return sock;

fail:
    log_write(CRITICAL, "Error creating socketing");
    return NULL;
}

SocketAddr* socket_get_address(Socket* socket)
{
    return (SocketAddr*)&socket->addr;
}

SocketAddr* socket_address_create(const char* ip, const char* port)
{
    SocketAddr* sock_addr = st_malloc(sizeof(SocketAddr));
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(port));
    inet_pton(AF_INET, ip, &addr.sin_addr);
    sock_addr->addr = addr;
    return sock_addr;
}

void socket_address_destroy(SocketAddr* addr)
{
    st_free(addr);
}

static void socket_set_ip_and_port(Socket* sock)
{
    socklen_t len = sizeof(struct sockaddr);
    getsockname(sock->fd, (struct sockaddr*)&sock->addr, &len);
    if (sock->ip != NULL)
        string_free(sock->ip);
    sock->ip = string_create("%s", inet_ntoa(sock->addr.sin_addr));
    if (sock->port != NULL)
        string_free(sock->port);
    sock->port = string_create("%d", ntohs(sock->addr.sin_port));
}

bool socket_bind(Socket* sock)
{
    bool result = bind(sock->fd, (struct sockaddr*)&sock->addr, sizeof(sock->addr)) != -1;
    socket_set_ip_and_port(sock);
    return result;
}

bool socket_listen(Socket* sock)
{
    return listen(sock->fd, 2) == 0;
}

Socket* socket_accept(Socket* sock)
{
    Socket* new_sock;
    socklen_t addrlen;
    int fd;
    addrlen = sizeof(sock->addr);
    fd = accept(sock->fd, (struct sockaddr*)&sock->addr, &addrlen);
    if (fd == -1)
        return NULL;
    new_sock = get_free_socket(sock->ctx);
    if (new_sock == NULL) {
        shutdown(fd, SHUT_RDWR);
        return NULL;
    }
    new_sock->fd = fd;
    new_sock->connected = true;
    socket_set_ip_and_port(new_sock);
    //new_sock->ip = string_create("%s", inet_ntoa(sock->addr.sin_addr));
    //new_sock->port = string_create("%d", ntohs(sock->addr.sin_port));

    return new_sock;
}

bool socket_connect(Socket* sock)
{
    sock->connected = connect(sock->fd, (struct sockaddr*)&sock->addr, sizeof(sock->addr)) == 0;
    return sock->connected;
}

bool socket_connected(Socket* socket)
{
    return socket->connected;
}

void socket_destroy(Socket* sock)
{
    NetContext* ctx;
    ctx = sock->ctx;
    pthread_mutex_lock(&ctx->mutex);
    if (sock->fd != -1) {
        shutdown(sock->fd, SHUT_RDWR);
        close(sock->fd);
    }
    if (sock == ctx->head)
        ctx->head = sock->next;
    else
        sock->prev->next = sock->next;
    if (sock == ctx->tail)
        ctx->tail = sock->prev;
    else
        sock->next->prev = sock->prev;
    if (sock->ip != NULL)
        string_free(sock->ip);
    if (sock->port != NULL)
        string_free(sock->port);
    st_free(sock);
    pthread_mutex_unlock(&ctx->mutex);
}

bool socket_send(Socket* sock, Packet* packet)
{
    return send(sock->fd, packet->buffer - PACKET_HEADER_BYTES, packet->length + PACKET_HEADER_BYTES, 0) != -1;
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
    return sendto(src_socket->fd, 
                  packet->buffer - PACKET_HEADER_BYTES, 
                  packet->length + PACKET_HEADER_BYTES, 
                  0, 
                  (struct sockaddr*)&dst_addr->addr, 
                  sizeof(dst_addr->addr)) != -1;
}

Packet* socket_recv(Socket* sock)
{
    Packet* packet;
    ssize_t length;
    ssize_t received = 0;
    char buffer[PACKET_HEADER_BYTES];
    while (received < PACKET_HEADER_BYTES) {
        length = read(sock->fd, buffer + received, PACKET_HEADER_BYTES - received);
        if (length == -1) {
            log_write(CRITICAL, "read failed: errno = %d", errno);
            return NULL;
        }
        received += length;
    }

    packet = st_malloc(sizeof(Packet));
    memcpy(&packet->length, buffer, sizeof(packet->length));
    memcpy(&packet->id, buffer + sizeof(packet->length), sizeof(packet->id));
    packet->buffer = st_malloc((packet->length + PACKET_HEADER_BYTES) * sizeof(char));
    memcpy(packet->buffer, buffer, PACKET_HEADER_BYTES);

    received = 0;
    while (received < (ssize_t)packet->length) {
        length = read(sock->fd, packet->buffer + PACKET_HEADER_BYTES + received, packet->length - received);
        if (length == -1) {
            log_write(CRITICAL, "read failed: errno = %d", errno);
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
    ssize_t len = recvfrom(src_socket->fd, buffer, sizeof(buffer), 0, (struct sockaddr*)&(*dst_addr)->addr, &client_len);
    if (len <= 0) {
        log_write(CRITICAL, "recvfrom failed: errono = %d", errno);
        st_free(*dst_addr);
        return NULL;
    }
    packet = st_malloc(sizeof(Packet));
    memcpy(&packet->length, buffer, sizeof(packet->length));
    memcpy(&packet->id, buffer + sizeof(packet->length), sizeof(packet->id));
    packet->buffer = st_malloc((packet->length + PACKET_HEADER_BYTES) * sizeof(char));
    memcpy(packet->buffer, buffer, packet->length + PACKET_HEADER_BYTES);
    packet->buffer += PACKET_HEADER_BYTES;
    return packet;
}

void socket_set_thread_id(Socket* sock, pthread_t thread_id)
{
    sock->has_thread = true;
    sock->thread_id = thread_id;
}

const char* socket_ip(Socket* socket)
{
    return socket->ip;
}

const char* socket_port(Socket* socket)
{
    return socket->port;
}

#endif
