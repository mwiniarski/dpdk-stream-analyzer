#ifndef __CONTAINERS__
#define __CONTAINERS__

#include <rte_mbuf.h>

struct Buffer 
{
    static const int CAPACITY = 32;

    int size;
    rte_mbuf *data[CAPACITY];

    rte_mbuf* operator [] (int index) { return data[index]; }
};

struct Packet
{
    Packet(rte_mbuf *mbuf) 
    {
        size = mbuf->data_len;
        data = (char*) mbuf->buf_addr + mbuf->data_off;
    }

    int size;
    void *data;
};

#endif