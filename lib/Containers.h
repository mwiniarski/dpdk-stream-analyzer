#ifndef __CONTAINERS__
#define __CONTAINERS__

#include <rte_mbuf.h>

/**
 * Wrapper for an array of rte_mbuf pointers and its size.
 */
struct MBuffer
{
    static const int CAPACITY = 32;

    // Current number of used pointers
    int size;

    rte_mbuf *data[CAPACITY];
    rte_mbuf* operator [] (int index) { return data[index]; }
};

/**
 * Wrapper for pointer to data stored in mbuf and its size.
 * Used as temporary to pass data to callback.
 */
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