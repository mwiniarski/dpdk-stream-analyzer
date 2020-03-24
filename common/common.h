#ifndef _COMMON_H_
#define _COMMON_H_

#include <rte_eal.h>
#include <rte_ethdev.h>

#include <string>

#define MZ_PORT_INFO "PORT_INFO"

#define RING_NAME_PREFIX "RING_"

#define MBUF_SIZE 32

#define MAX_PORT_COUNT 8

struct PortInfo {
    uint16_t portCount;
    uint16_t rxID;
    uint16_t txID;
};


void initEAL(int &argc, char **argv[]);

std::string getRingName(uint index);

void sendFromEthToRing(int port, rte_ring *ring);
void sendFromRingToEth(rte_ring *ring, int port);
void sendFromRingToRing(rte_ring *rxRing, rte_ring *txRing);

#endif