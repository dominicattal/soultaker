#ifndef NETWORKING_H
#define NETWORKING_H

#include <stdbool.h>
#include <stdlib.h>

#define BIT_TCP  0x1
#define BIT_UDP  0x2

typedef enum {
    PACKET_TEST
} PacketEnum;

typedef struct {
    PacketEnum id;
    char* buffer;
    size_t length;
} Packet;

// OS dependent structs
typedef struct Socket Socket;
typedef struct NetContext NetContext;

// initialize a net context. each net context has their own singly-linked list of in use sockets
NetContext* networking_init(void);

// shutdown all of the sockets in the netcontext. this will unblock all listening sockets
void        networking_shutdown_sockets(NetContext* ctx);

// join all the sockets' threads if a thread was set using socket_set_thread_id
void        networking_join_sockets(NetContext* ctx);

// cleanup networking context
void        networking_cleanup(NetContext* ctx);

// for windows, doesn't do anything on linux
int         networking_get_last_error(void);

// Create a new socket
// ip   -> ip to create socket for. NULL to create on host
// port -> number from 0-65535 in string format. Undefined if NULL.
// tcp  -> whether the socket support tcp or udp
Socket* socket_create(NetContext* ctx, const char* ip, const char* port, int flags);

// Bind a socket so clients can access. Returns true if successful
bool    socket_bind(Socket* socket);

// Listen for incoming connection requests. Returns true if successful
bool    socket_listen(Socket* socket);

// Accept a connection request. Returns the socket if successful, NULL otherwise
Socket* socket_accept(Socket* socket);

// Connect to a socket. Returns true if successful
bool    socket_connect(Socket* socket);

// Destroy socket and all related information
void    socket_destroy(Socket* socket);

// Check if socket still connected. Returns true if successful
bool    socket_connected(Socket* socket);

// Send a packet over a socket. Returns true if successful
bool    socket_send(Socket* socket, Packet* packet);

// Send packet to all connected clients in context
void    socket_send_all(NetContext* ctx, Packet* packet);

// Receive a packet from a socket
Packet* socket_recv(Socket* socket);

// Keep track of a socket's handler thread. 
void    socket_set_thread_id(Socket* socket, pthread_t thread_id);

// Create a packet with id with a buffer of length. buffer can be NULL iff length is 0.
// Returns NULL if buffer is NULL and length is not 0
Packet* packet_create(PacketEnum id, int length, const char* buffer);

// Frees memory from packet
void    packet_destroy(Packet* packet);

#endif
