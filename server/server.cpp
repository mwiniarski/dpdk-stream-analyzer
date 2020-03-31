#include <unistd.h>

#include <iostream>
#include <vector>

#include <shared.h>

using namespace std;


rte_mempool* createMemPool()
{
    const char* MBUF_POOL_NAME = "MBUF_POOL";
    const int   MBUF_CACHE_SIZE = 250;
    const int   MBUF_COUNT = 8019;

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

vector<Ring> initRings(GlobalInfo *gi)
{
    vector<Ring> firstRings;
    for (int i = 0; i < gi->chainCount; i++)
    {
        // First rings are needed by server, rest is irrelevant
        firstRings.emplace_back(0, i, true);

        for (int j = 1; j < gi->appsInChain[i]; j++)
        {
            Ring(j, i, true);
        }
    }

    return firstRings;
}

void newPacketCallback(Packet &&p)
{
    static int c = 1;

    if (c++ % 1000 == 0)
    {
        Logl("Server " << c);
    }
}

int main(int argc, char *argv[])
{
    initEAL(argc, &argv);

    if (argc < 2)
        rte_exit(EXIT_FAILURE, "Usage: ./server <chain_size_0> [<chain_size_1>...]\n");

    vector<int> chainSizes;
    for (int i = 1; i < argc; i++)
        chainSizes.push_back(stoi(argv[i]));

    // Take app count as program argument
    GlobalInfo *info = GlobalInfo::init(chainSizes);

    // Create memory pool
    rte_mempool* mp = createMemPool();

    // Initialize eth ports
    Port rxPort(0, mp);
    Port txPort(1, mp);

    // Initialize memory rings for all apps
    vector<Ring> firstRings = initRings(info);

    // Initialize statistics ring
    Ring(info->STATS_RING);

    // Create senders
    vector<Sender> ethToRingSenders;
    for (Ring &ring : firstRings)
        ethToRingSenders.emplace_back(rxPort,
                                      ring,
                                      newPacketCallback);

    Sender ethToEth(txPort, rxPort, newPacketCallback);

    for (;;)
    {
        // Super simple time-based round-robin
        for (Sender &ethToRing : ethToRingSenders)
        {
            // ETH --> RING[i]
            ethToRing.sendPacketBurst();

            // ETH <-- ETH
            ethToEth.sendPacketBurst();

            usleep(1000);
        }
    }
}