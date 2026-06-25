#include "net.h"
#include "malloc.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

// compatible on linux and windows

/*
   4 bytes -> length of packet
   2 bytes -> id of packet
   length bytes -> buffer
*/

Packet* packet_create(u32 id, i32 length, const char* buffer)
{
    Packet* packet;
    if (buffer == NULL && length != 0)
        return NULL;
    packet = st_malloc(sizeof(Packet));
    packet->id = id;
    packet->length = length;
    packet->buffer = st_malloc((length + PACKET_HEADER_BYTES) * sizeof(char));
    memcpy(packet->buffer, &length, sizeof(length));
    memcpy(packet->buffer + sizeof(length), &id, sizeof(length));
    if (buffer != NULL)
        memcpy(packet->buffer + PACKET_HEADER_BYTES, buffer, length);
    packet->buffer += PACKET_HEADER_BYTES;
    return packet;
}

void packet_destroy(Packet* packet)
{
    st_free(packet->buffer - PACKET_HEADER_BYTES);
    st_free(packet);
}
