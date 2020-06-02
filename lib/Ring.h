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
     * Create new or find existing ring based on position in system.
     * Use this constructor for in-chain rings.
     *
     * @param appIndex   Index of the app in its chain
     * @param chainIndex Index of the chain
     * @param createNew  If true, create new ring, lookup otherwise
     */
    Ring(int appIndex, int chainIndex, bool createNew = false);

    /**
     * Create new ring with given name. Use this for initialization of
     * global rte_ring only.
     *
     * @param name  Name of new ring.
     */
    Ring(const std::string& name);

    // See Device.h
    void getPackets(MBuffer &buffer) override;
    int sendPackets(MBuffer &buffer) override;

    // Get number of elements in the ring
    int size();

private:
    // Setup ring with a name. See Ring(string,bool)
    void init(const std::string& name, bool createNew);

    // Create new ring with given name
    void create(const std::string& name);

    // Find existing ring with given name
    void lookup(const std::string& name);

    // Create a universal ring name from chain and app indices
    std::string getName(int chainNumber, int chainIndex);

    // Pointer to underlying ring structure
    rte_ring *_ring;
};

#endif