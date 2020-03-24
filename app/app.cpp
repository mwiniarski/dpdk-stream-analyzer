#include <iostream>
#include <unistd.h>

#include <rte_memzone.h>
#include <vector>

#include <common/common.h>

using namespace std;

GlobalInfo* getGlobalInfo()
{
    // Retrieve port info shared by server
    const rte_memzone *memzone = rte_memzone_lookup(GLOBAL_INFO_NAME);

    if (!memzone)
        rte_exit(EXIT_FAILURE, "ERROR: Can't read '%s' memzone\n", GLOBAL_INFO_NAME);

    return (GlobalInfo *) memzone->addr;
}

rte_ring* getRing(uint index)
{
    rte_ring* ring;

    string ringName = getRingName(index);

    // Lookup ring
    ring = rte_ring_lookup(ringName.c_str());

    if (!ring)
        rte_exit(EXIT_FAILURE, "ERROR: Can't find ring [%s]\n", ringName.c_str());

    return ring;
}

int main(int argc, char* argv[])
{
    initEAL(argc, &argv);

    if (argc < 2)
        rte_exit(EXIT_FAILURE, "Usage: ./app <index>\n");

    // Retrieve global info
    GlobalInfo* globalInfo = getGlobalInfo();

    // Retrieve rx ring used by app
    int appIndex = stoi(argv[1]);
    rte_ring* rxRing = getRing(appIndex);

    // Set tx ring if this is not the last app in chain
    rte_ring* txRing = NULL;
    if (appIndex + 1 < globalInfo->appCount)
    {
        txRing = getRing(appIndex + 1);
    }

    if (txRing)
    {
        Logl(">>> MIDDLE app mode");

        for (;;)
        {
            // Sleep 1ms
            usleep(1000);

            // RING --> RING
            sendFromRingToRing(rxRing, txRing);
        }
    }
    else
    {
        Logl(">>> LAST-IN-CHAIN app mode");

        for (;;)
        {
            // Sleep 1ms
            usleep(1000);

            // RING --> ETH
            sendFromRingToEth(rxRing, globalInfo->txPort);
        }
    }

}