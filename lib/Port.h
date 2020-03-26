#ifndef __PORT__
#define __PORT__

#include <rte_ethdev.h>

#include "Device.h"

/**
 * Class wrapping ethernet port. Used to get/send
 * packets from/to port.
 */
class Port : public Device
{
    static const int ETH_RING_SIZE = 512;

public:
    /**
     * Create port wrapper.
     *
     * @param port  Eth port number
     * @param mp    Used for initialization. Should only be passed by server.
     */
    Port(int port, rte_mempool* mp = NULL);

    // See Device.h
    void getPackets(Buffer &buffer) override;
    void sendPackets(Buffer &buffer) override;

private:
    /**
     * Used by constructor to initialize rte_eth port.
     */
    int init(rte_mempool *mbufPool);

private:
    int _port;
};

#endif