#ifndef __RING__
#define __RING__

#include <string>

#include <rte_ring.h>

#include "Device.h"

/**
 * Class wrapping rte_ring. Used to get/send
 * packets from/to ring.
 */
class Ring : public Device
{
    // Name prepended to all ring names
    static const std::string PREFIX;

    // Size of all rings
    static const int SIZE;

public:
    /**
     * Create new or find existing ring.
     *
     * @param appIndex   Index of the app in its chain
     * @param chainIndex Index of the chain
     * @param createNew  If true, create new ring, lookup otherwise
     */
    Ring(uint appIndex, uint chainIndex, bool createNew = false);

    // See Device.h
    void getPackets(Buffer &buffer) override;
    void sendPackets(Buffer &buffer) override;

private:
    std::string getName(uint chainNumber, uint chainIndex);

    rte_ring *_ring;
};

#endif