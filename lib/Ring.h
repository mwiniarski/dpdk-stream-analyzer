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
    Ring(int index, bool createNew = false);

    void getPackets(Buffer &buffer) override;
    void sendPackets(Buffer &buffer) override;

private:
    std::string getName(uint index);

    rte_ring *_ring;
};

#endif