#ifndef __RING__
#define __RING__

#include <string>

#include <rte_ring.h>

#include "Device.h"

class Ring : public Device
{
    static const std::string PREFIX;
    static const int SIZE;

public:
    Ring(uint appIndex, uint chainIndex, bool createNew = false);

    void getPackets(Buffer &buffer) override;
    void sendPackets(Buffer &buffer) override;

private:
    std::string getName(uint chainNumber, uint chainIndex);

    rte_ring *_ring;
};

#endif