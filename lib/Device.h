#ifndef __DEVICE__
#define __DEVICE__

#include <utility>

#include "Containers.h"

/**
 * Interface for Port and Ring classes.
 */
class Device
{
public:
    Device(int chainInd, int appInd)
        :_chainIndex(chainInd),
         _appIndex(appInd)
    {}

    Device() {}

    /**
     * Retrieve packets into passed buffer.
     *
     * @param buffer[out] Will be filled with data and size will be set
     * to number of packets.
     */
    virtual void getPackets(MBuffer &buffer) = 0;

    /**
     * Send buffer.size of packets stored in the buffer.data.
     *
     * @param buffer[in] Packets to be sent.
     */
    virtual void sendPackets(MBuffer &buffer) = 0;

    /**
     * Get chain and app indices regarding to the system.
     *
     * @return (chain index, app index)
     */
    std::pair<int, int> getPosition() { return std::make_pair(_chainIndex, _appIndex); };

    /**
     * Check if device is and app.
     *
     * @return true if app, false if eth
     */
    bool isApp() { return _chainIndex != -1; }

protected:

    // Packets dropped while sending to device
    int _dropped = 0;

    // Position in the system
    int _chainIndex;
    int _appIndex;
};

#endif