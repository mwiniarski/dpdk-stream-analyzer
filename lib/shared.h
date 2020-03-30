#ifndef __SHARED__
#define __SHARED__

#include <iostream>

#include <rte_eal.h>

#include "Log.h"
#include "Port.h"
#include "Ring.h"
#include "Sender.h"

/**
 * Initialize DPDK and remove dpdk-related main() arguments.
 */
void initEAL(int &argc, char **argv[])
{
    int ret = rte_eal_init(argc, *argv);
    if (ret < 0)
        rte_exit(EXIT_FAILURE, "ERROR: Can't init EAL\n");

    argc -= ret;
    *argv += ret;
}
#endif