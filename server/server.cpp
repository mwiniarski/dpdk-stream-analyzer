#include <unistd.h>

#include <iostream>
#include <vector>

#include "common/common.h"

// === GLOBALS
#define MBUF_COUNT 8192
#define MBUF_CACHE_SIZE 250
#define MBUF_POOL_NAME "MBUF_POOL"

#define ETH_RING_SIZE 512
#define RING_SIZE 128

// Structure stored in memory shared between processes
PortInfo *portInfo;

// Static memory to use by all aplications to pass packets
rte_mempool *mbufPool;

// ===

using namespace std;


int portInit(uint16_t port, rte_mempool *mbufPool)
{
    int ret;
    uint16_t nb_rxd = ETH_RING_SIZE;
    uint16_t nb_txd = ETH_RING_SIZE;

    // Configure eth device
    rte_eth_conf portConf;
    memset(&portConf, 0, sizeof(rte_eth_conf));
    portConf.rxmode.max_rx_pkt_len = ETHER_MAX_LEN;

    // Set 1 rx and 1 tx queue for given port
    ret = rte_eth_dev_configure(port, 1, 1, &portConf);
    if (ret != 0)
        return ret;

    // Check if descriptor numbers are correct
    ret = rte_eth_dev_adjust_nb_rx_tx_desc(port, &nb_rxd, &nb_txd);
    if (ret != 0)
        return ret;

    int sockId = rte_eth_dev_socket_id(port);

    // Setup rx queue
    ret = rte_eth_rx_queue_setup(port, 0, nb_rxd, sockId, NULL, mbufPool);
    if (ret < 0)
        return ret;

    // Setup tx queue
    ret = rte_eth_tx_queue_setup(port, 0, nb_txd, sockId, NULL);
    if (ret < 0)
        return ret;

    // Start eth device
    ret = rte_eth_dev_start(port);
    if (ret < 0)
        return ret;

    // Enable rx in promiscuous mode for eth device.
    rte_eth_promiscuous_enable(port);

    return 0;
}

void initEth()
{
    // Check ports
    int portCount = rte_eth_dev_count();

    if (portCount < 2)
        rte_exit(EXIT_FAILURE, "ERROR: At least 2 ports needed, found %d\n", portCount);

    cout << ">>> Found " << portCount << " ports available" << endl;

    // Keep port info in system-wise accessible way
    const rte_memzone *memZone = rte_memzone_reserve(
                                        MZ_PORT_INFO, 
                                        sizeof(PortInfo),
                                        rte_socket_id(),
                                        0);

    if (!memZone)
        rte_exit(EXIT_FAILURE, "ERROR: Can't reserve memory zone for port info\n");

    portInfo = (PortInfo*) memZone->addr;
    memset(portInfo, 0, sizeof(PortInfo));

    portInfo->portCount = portCount;
    portInfo->rxID = 0;
    portInfo->txID = 1;

    // Init mbuf pool - static memory for buffers to use by all apps
    mbufPool = rte_pktmbuf_pool_create(MBUF_POOL_NAME, 
                                       MBUF_COUNT,
                                       MBUF_CACHE_SIZE, 
                                       0, 
                                       RTE_MBUF_DEFAULT_BUF_SIZE, 
                                       rte_socket_id());

    if (!mbufPool)
            rte_exit(EXIT_FAILURE, "Error: Can't create mbuf pool\n");

    cout << ">>> Mbuf pool " << MBUF_POOL_NAME << 
            " [" << MBUF_COUNT << "] created" << endl;

    // Init eth ports
    for (int i = 0; i < 2; i++)
        portInit(i, mbufPool);
}

vector<rte_ring*> initRings(uint count)
{
    vector<rte_ring*> rings;
    for (uint i = 0; i < count; i++)
    {
        string ringName = getRingName(i);

        // Create ring
        rings.push_back(rte_ring_create(ringName.c_str(), RING_SIZE, rte_socket_id(), 0));

        if (!rings[rings.size() - 1])
            rte_exit(EXIT_FAILURE, "ERROR: Can't create ring [%s]\n", ringName.c_str());

        cout << ">>> Ring " << ringName << " ["
            << RING_SIZE << "] created" << endl;
    }

    return rings;
}

int main(int argc, char *argv[])
{
    initEAL(argc, &argv);

    initEth();

    vector<rte_ring*> rings = initRings(4);

    for (;;)
    {
        // Sleep 0.5 sec
        usleep(500000);

        sendFromEthToRing(portInfo->rxID, rings[0]);
        sendFromRingToEth(rings[1], portInfo->rxID);
    }
}