#ifndef __CONTAINERS__
#define __CONTAINERS__

#include <rte_mbuf.h>

/**
 * Wrapper for an array of rte_mbuf pointers and its size.
 */
struct MBuffer
{
    static const int CAPACITY = 8;

    // Current number of used pointers
    uint size;

    rte_mbuf* data[CAPACITY];
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
        data = (uint8_t*) mbuf->buf_addr + mbuf->data_off;
    }

    int size;
    uint8_t *data;
};

#endif