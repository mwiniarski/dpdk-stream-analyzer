#include <unistd.h>

#include <iostream>
#include <vector>

#include <shared.h>

using namespace std;

// === Settings
#define MBUF_POOL_NAME "MBUF_POOL"
#define MBUF_CACHE_SIZE 250
#define MBUF_COUNT 8192
// ===


rte_mempool* createMemPool()
{
    // Check ports
    int portCount = rte_eth_dev_count();

    if (portCount < 2)
        rte_exit(EXIT_FAILURE, "ERROR: At least 2 ports needed, found %d\n", portCount);

    Logl(">>> Found " << portCount << " ports available");

    // Init mbuf pool - static memory for buffers to use by all apps
    rte_mempool *mbufPool = rte_pktmbuf_pool_create(
                                        MBUF_POOL_NAME,
                                        MBUF_COUNT,
                                        MBUF_CACHE_SIZE,
                                        0,
                                        RTE_MBUF_DEFAULT_BUF_SIZE,
                                        rte_socket_id());

    if (!mbufPool)
            rte_exit(EXIT_FAILURE, "Error: Can't create mbuf pool\n");

    Logl(">>> Mbuf pool " << MBUF_POOL_NAME <<
            " [" << MBUF_COUNT << "] created");

    return mbufPool;
}

vector<Ring> initRings(uint count)
{
    vector<Ring> rings;
    for (uint i = 0; i < count; i++)
    {
        rings.emplace_back(i, true);

        Logl(">>> RING_" << i << " created");
    }

    return rings;
}

void newPacketCallback(Packet &&p)
{
    static int c = 0;
    Logl("Server " << c++);
}

int main(int argc, char *argv[])
{
    initEAL(argc, &argv);

    if (argc < 2)
        rte_exit(EXIT_FAILURE, "Usage: ./server <app_count>\n");

    // Take app count as program argument
    GlobalInfo *info = GlobalInfo::init(stoi(argv[1]));

    // Create memory pool
    rte_mempool* mp = createMemPool();

    // Initialize eth ports
    auto rxPort = Port(0, mp);
    auto txPort = Port(1, mp);

    // Initialize memory rings for all apps
    vector<Ring> rings = initRings(info->appCount);

    // Create senders
    Sender ethToRing(rxPort, rings[0], newPacketCallback);
    Sender ethToEth(txPort, rxPort, newPacketCallback);

    for (;;)
    {
        // Sleep 1ms
        usleep(1000);

        // ETH --> RING
        ethToRing.sendPacketBurst();

        // ETH <-- ETH
        ethToEth.sendPacketBurst();
    }
}