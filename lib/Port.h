#ifndef __PORT__
#define __PORT__

#include <rte_ethdev.h>

#include "Device.h"

class Port : public Device
{
    static const int ETH_RING_SIZE = 512;

public:
    Port(int port);
    Port(int port, rte_mempool* mp);

    void getPackets(Buffer &buffer) override;
    void sendPackets(Buffer &buffer) override;

private:
    int init(rte_mempool *mbufPool);

private:
    int _port;
};

#endif