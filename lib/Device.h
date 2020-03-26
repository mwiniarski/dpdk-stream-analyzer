#ifndef __DEVICE__
#define __DEVICE__

#include <cstdint>

#include "Containers.h"

class Device
{
public:
    virtual void getPackets(Buffer &buffer) = 0;
    virtual void sendPackets(Buffer &buffer) = 0;

protected:

    // Packets dropped while sending to device
    int _dropped = 0;
};

#endif