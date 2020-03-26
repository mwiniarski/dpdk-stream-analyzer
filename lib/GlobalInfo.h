#ifndef __GLOBALINFO__
#define __GLOBALINFO__

#include <cstdint>
#include <string>
#include <cstring>

#include <rte_memzone.h>
#include <rte_lcore.h>

struct GlobalInfo 
{
    static const std::string NAME;

    static GlobalInfo* init(int appCount);
    static GlobalInfo* get();

    // Ethernet ports
    uint16_t rxPort;
    uint16_t txPort;

    // Number of apps in chain
    uint16_t appCount;
};


#endif