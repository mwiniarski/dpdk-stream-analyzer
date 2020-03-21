#ifndef _COMMON_H_
#define _COMMON_H_

#include <rte_eal.h>
#include <rte_ethdev.h>

#define MZ_PORT_INFO "PORT_INFO"

#define MAX_PORT_COUNT 8

struct PortInfo {
    uint16_t portCount;
    uint16_t portID[MAX_PORT_COUNT];
};


void initEAL(int &argc, char **argv[]);

#endif