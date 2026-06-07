#include "net.h"
#include "malloc.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

#define PACKET_HEADER_BYTES 6

// compatible on linux and windows

/*
   4 bytes -> length of packet
   2 bytes -> id of packet
   length bytes -> buffer
*/

Packet* packet_create(u32 id, int length, const char* buffer)
{
    Packet* packet;
    if (buffer == NULL && length != 0)
        return NULL;
    packet = st_malloc(sizeof(Packet));
    packet->id = id;
    packet->length = length + PACKET_HEADER_BYTES;
    packet->buffer = st_malloc(packet->length * sizeof(char));
    packet->buffer[0] = (length>>24) & 0xFF;
    packet->buffer[1] = (length>>16) & 0xFF;
    packet->buffer[2] = (length>>8) & 0xFF;
    packet->buffer[3] = length & 0xFF;
    packet->buffer[4] = (id>>8) & 0xFF;
    packet->buffer[5] = id & 0xFF;
    memcpy(packet->buffer+PACKET_HEADER_BYTES, buffer, length);
    return packet;
}

void packet_destroy(Packet* packet)
{
    st_free(packet->buffer);
    st_free(packet);
}
