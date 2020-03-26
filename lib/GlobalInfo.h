#ifndef __GLOBALINFO__
#define __GLOBALINFO__

#include <cstdint>
#include <string>
#include <cstring>
#include <vector>

#include <rte_memzone.h>
#include <rte_lcore.h>

struct GlobalInfo 
{
    static const std::string NAME;
    static const int MAX_CHAINS = 8;
    static const int MAX_APPS = 8;

    static GlobalInfo* init(std::vector<int> chainSizes);
    static GlobalInfo* get();

    // Helpers
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