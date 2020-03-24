#include <iostream>
#include <unistd.h>

#include <rte_memzone.h>
#include <vector>

#include <common/common.h>

using namespace std;

// === GLOBALS

// Structure stored in memory shared between processes
PortInfo *portInfo;

// ===

void initEth()
{
    // Retrieve port info shared by server
    const rte_memzone *memzone = rte_memzone_lookup(MZ_PORT_INFO);

    if (!memzone)
        rte_exit(EXIT_FAILURE, "ERROR: Cannot get port info structure\n");

    portInfo = (PortInfo *) memzone->addr;

    cout << ">>> TX eth port: " << portInfo->txID << endl;
}

vector<rte_ring*> lookupRings(uint count)
{
    // Find all rings with given names
    vector<rte_ring*> rings;
    for (uint i = 0; i < count; i++)
    {
        string ringName = getRingName(i);

        // Lookup ring
        rings.push_back(rte_ring_lookup(ringName.c_str()));

        if (!rings[rings.size() - 1])
            rte_exit(EXIT_FAILURE, "ERROR: Can't find ring [%s]\n", ringName.c_str());
    }

    return rings;
}

int main(int argc, char* argv[])
{
    initEAL(argc, &argv);

    initEth();

    vector<rte_ring*> rings = lookupRings(2);

    if (argc > 1 && stoi(argv[1]) == 0)
    {
        cout << ">>> MIDDLE app mode" << endl;

        for (;;)
        {
            // Sleep 1ms
            usleep(1000);

            // RING --> RING
            sendFromRingToRing(rings[0], rings[1]);
        }
    }
    else
    {
        cout << ">>> LAST-IN-CHAIN app mode" << endl;

        for (;;)
        {
            // Sleep 1ms
            usleep(1000);

            // RING --> ETH
            sendFromRingToEth(rings[1], portInfo->txID);
        }
    }

}