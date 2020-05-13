#ifndef __SENDER__
#define __SENDER__

#include <memory>
#include <functional>

#include "Device.h"
#include "Statistics.h"

/**
 * Class used to transfer packets from one Device to another. Inbetween
 * user-defined callback is called.
 */
class Sender
{
public:
    /**
     * Setup connection between rx and dx with function called inbetween.
     *
     * @param rx     Device to receive packets from
     * @param tx     Device to send packets to
     * @param cb     Function called between packet transfers
     */
    Sender(Device& rx,
           Device& tx,
           std::function<void (Packet &&p)> cb);

    /**
     * Perform one transfer of all packets accumulated on rx.
     */
    int sendPacketBurst();

private:

    // Save timers of packets and send them if needed
    void collectStats();

    // Array of mbufs used to transfer packets
    MBuffer _buffer;

    // Devices - can be either port or ring
    Device& _rxDevice;
    Device& _txDevice;

    // Function called in the middle of transfer
    std::function<void (Packet &&p)> _callback;

    // Stats
    std::unique_ptr<Statistics> _stats;
};

#endif