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
    // TODO
    // First draft of message header passed when sending stats
    struct MessageHeader
    {
        enum Type { ETH = 0, APP = 1};

        Type reporter;    // 0 - eth, 1 - app
        int dataLength;   // In bytes
        int chainIndex;
        int appIndex;
        uint8_t* data;
    };

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
     * Create new or find existing ring based on name.
     * Use this constructor for message sending rings (like statistics).
     * It accesses global mempool.
     *
     * @param name      Name of the ring
     * @param createNew If true, create new ring, lookup otherwise
     */
    Ring(const std::string& name, bool createNew = false);

    // See Device.h
    void getPackets(MBuffer &buffer) override;
    void sendPackets(MBuffer &buffer) override;

    // TODO
    void sendMessage(const MessageHeader& mh);

private:
    // Setup ring with a name. See Ring(string,bool)
    void init(const std::string& name, bool createNew);

    // Create new ring with given name
    void create(const std::string& name);

    // Find existing ring with given name
    void lookup(const std::string& name);

    // Get pointer to existing mempool based on GlobalInfo.
    void getMempool();

    // Create a universal ring name from chain and app indices
    std::string getName(int chainNumber, int chainIndex);

    // Pointer to underlying ring structure
    rte_ring *_ring;

    // Pointer to mempool used to send messages
    rte_mempool *_mempool;
};

#endif