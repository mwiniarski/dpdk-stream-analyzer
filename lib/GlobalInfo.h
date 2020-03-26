#ifndef __GLOBALINFO__
#define __GLOBALINFO__

#include <cstdint>
#include <string>
#include <cstring>
#include <vector>

#include <rte_memzone.h>
#include <rte_lcore.h>

/**
 * This is an overlay for memory on hugepages shared between all the programs.
 * The server runs init() and all the apps do get(). It contains information
 * about the layout of chains to allow apps to make decision where to forward
 * packets.
 */
struct GlobalInfo 
{
    // String identifier of shared memory
    static const std::string NAME;
    static const int MAX_CHAINS = 8;
    static const int MAX_APPS = 8;

    /**
     * Initialize shared memory with data about chains. Used by server.
     *
     * @param chainSizes Vector of lengths of consecutive chains.
     * [2,3] would create two chains one of size 2 and other of size 3.
     *
     * @return Pointer to shared memory. Doesn't need to be deallocated.
     */
    static GlobalInfo* init(std::vector<int> chainSizes);

    /**
     * Get pointer to already initialized shared memory, used by apps.
     *
     * @return Pointer to shared memory. Doesn't need to be deallocated.
     */
    static GlobalInfo* get();


    // Helpers

    /**
     * Calculate if app with given index and chain index is the last in
     * the chain based on the current information stored in shared memory.
     */
    bool isLastInChain(uint appIndex, uint chainIndex);

    // Values that influence size of struct

    // Ethernet ports
    uint16_t rxPort;
    uint16_t txPort;

    // Chain values
    uint16_t chainCount;
    uint16_t appsInChain[MAX_CHAINS];
};


#endif