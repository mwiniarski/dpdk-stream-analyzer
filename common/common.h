#ifndef _COMMON_H_
#define _COMMON_H_

#include <rte_eal.h>
#include <rte_ethdev.h>

#include <string>

#ifdef LOG
    #define Log(x)  (std::cout << x)
#else
    #define Log(x) do{}while(0)
#endif // LOG
#define Logl(x) Log(x << std::endl)

#define GLOBAL_INFO_NAME "GLOBAL_INFO"

#define RING_NAME_PREFIX "RING_"

#define MBUF_SIZE 32

#define MAX_PORT_COUNT 8

struct GlobalInfo {

    // Ethernet ports
    uint16_t rxPort;
    uint16_t txPort;

    // Number of apps in chain
    uint16_t appCount;
};


void initEAL(int &argc, char **argv[]);

std::string getRingName(uint index);

void sendFromEthToEth(int rxPort, int txPort);
void sendFromEthToRing(int port, rte_ring *ring);
void sendFromRingToEth(rte_ring *ring, int port);
void sendFromRingToRing(rte_ring *rxRing, rte_ring *txRing);

#endif