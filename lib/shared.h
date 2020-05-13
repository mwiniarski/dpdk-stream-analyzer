#ifndef __SHARED__
#define __SHARED__

#include <iostream>

#include <rte_eal.h>

#include "Log.h"
#include "Port.h"
#include "Ring.h"
#include "Sender.h"

/**
 * Common sleep for app and server
 */
void mic_sleep(int loopsBeforeSwitch)
{
    static uint64_t counter = 0;
    // timespec t = {0, 1};
    // nanosleep(&t, NULL);

    if (++counter % loopsBeforeSwitch == 0)
        sched_yield();
}


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


/**
 * Set real-time scheduling of current thread.
 */
void schedule(int policy)
{
    sched_param sp = { .sched_priority = 1 };

    pthread_setschedparam(pthread_self(), policy, &sp);
    pthread_getschedparam(pthread_self(), &policy, &sp);

    Logl(">>> Policy = " <<
        ((policy == SCHED_FIFO)  ? "SCHED_FIFO" :
        (policy == SCHED_RR)    ? "SCHED_RR" :
        "SCHED_OTHER") << " priority = " << sp.sched_priority);

    sleep(3);
}

/**
 * Perform some artificial work related to packet size.
 * 
 * Work amount = O(packet.size * repeats)
 */
int calcPacketHash(Packet& packet, int repeats)
{
    int hash = 0;
    for (int r = 0; r < repeats; r++)
    {
        for (int i = 0; i < packet.size; i++)
        {
            hash += packet.data[i];
        }
    }

    return hash;
}

#endif