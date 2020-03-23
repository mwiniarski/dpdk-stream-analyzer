#include <unistd.h>

#include <iostream>
#include <string>

#include "common/common.h"

// === GLOBALS
#define MBUF_COUNT 8192
#define MBUF_CACHE_SIZE 250
#define MBUF_POOL_NAME "MBUF_POOL"

#define ETH_RING_SIZE 512
#define RING_SIZE 128

#define TX_RING_NAME RING_NAME_1
#define RX_RING_NAME RING_NAME_2

// Structure stored in memory shared between processes
PortInfo *portInfo;

// Static memory to use by all aplications to pass packets
rte_mempool *mbufPool;

// First application rings
rte_ring *txRing;
rte_ring *rxRing;
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

void initRings()
{
    // TX ring
    txRing = rte_ring_create(TX_RING_NAME, RING_SIZE, rte_socket_id(), 0);

    if (!txRing)
        rte_exit(EXIT_FAILURE, "ERROR: Problem with creating send ring\n");

    cout << ">>> Ring " << TX_RING_NAME << " ["
         << RING_SIZE << "] created" << endl;

    // RX ring
    rxRing = rte_ring_create(RX_RING_NAME, RING_SIZE, rte_socket_id(), 0);

    if (!rxRing)
        rte_exit(EXIT_FAILURE, "ERROR: Problem with creating send ring\n");

    cout << ">>> Ring " << RX_RING_NAME << " ["
         << RING_SIZE << "] created" << endl;
}

void forwardPackets()
{
    rte_mbuf *bufs[MBUF_SIZE];

    // Get packets from eth
    int rxCount = rte_eth_rx_burst(portInfo->rxID, 0, bufs, MBUF_SIZE);

    if (rxCount == 0)
        return;

    // Send to ring
    if(rte_ring_sp_enqueue_bulk(txRing, (void**) bufs, rxCount, NULL) == 0)
    {
        // Failed to send - packets are dropped
        for (int i = 0; i < rxCount; i++)
            rte_pktmbuf_free(bufs[i]);

        cout << "> Some packets [" << rxCount << "] were dropped!" << endl;
    }
    else
        cout << "> Packets sent: " << rxCount << endl;
}

void backwardPackets()
{
    rte_mbuf *bufs[MBUF_SIZE];

    // Get packets from ring
    int rxCount = rte_ring_dequeue_burst(rxRing, (void**) bufs, MBUF_SIZE, NULL);

    if (rxCount == 0)
        return;

    cout << "> Received [" << rxCount << "] packets from [" << RX_RING_NAME << "]";

    // Send goddamn packts
    int txCount = rte_eth_tx_burst(portInfo->rxID, 0, bufs, rxCount);

    // Free mbufs that were not sent
    if (txCount != rxCount)
    {
        for (int i = txCount; i < rxCount; i++)
            rte_pktmbuf_free(bufs[i]);

        cout << " and DROPPED [" << rxCount - txCount << "] of them!" << endl;
    }
    else
        cout << " and sent all of them." << endl;
}

int main(int argc, char *argv[])
{
    initEAL(argc, &argv);

    initEth();

    initRings();

    for (;;)
    {
        // Sleep 0.5 sec
        usleep(500000);

        forwardPackets();
        backwardPackets();
    }
}