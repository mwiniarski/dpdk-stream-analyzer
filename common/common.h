#ifndef _COMMON_H_
#define _COMMON_H_

#include <rte_eal.h>
#include <rte_ethdev.h>

#define MZ_PORT_INFO "PORT_INFO"

#define RING_NAME_1 "RING_1"
#define RING_NAME_2 "RING_2"

#define MBUF_SIZE 32

#define MAX_PORT_COUNT 8

struct PortInfo {
    uint16_t portCount;
    uint16_t rxID;
    uint16_t txID;
};


void initEAL(int &argc, char **argv[]);

#endif