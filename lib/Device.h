#ifndef __DEVICE__
#define __DEVICE__

#include <cstdint>

#include "Containers.h"

/**
 * Interface for Port and Ring classes.
 */
class Device
{
public:

    /**
     * Retrieve packets into passed buffer.
     *
     * @param buffer[out] Will be filled with data and size will be set
     * to number of packets.
     */
    virtual void getPackets(Buffer &buffer) = 0;

    /**
     * Send buffer.size of packets stored in the buffer.data.
     *
     * @param buffer[in] Packets to be sent.
     */
    virtual void sendPackets(Buffer &buffer) = 0;

protected:

    // Packets dropped while sending to device
    int _dropped = 0;
};

#endif