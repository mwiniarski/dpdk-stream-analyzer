#include "common.h"

#include <iostream>

void initEAL(int &argc, char **argv[])
{
    int ret = rte_eal_init(argc, *argv);
    if (ret < 0)
        rte_exit(EXIT_FAILURE, "ERROR: Can't init EAL\n");

    argc -= ret;
    *argv += ret;
}

std::string getRingName(uint index)
{
    return (RING_NAME_PREFIX + std::to_string(index));
}


void sendFromEthToRing(int port, rte_ring *ring)
{
    rte_mbuf *bufs[MBUF_SIZE];

    // Get packets from eth
    int rxCount = rte_eth_rx_burst(port, 0, bufs, MBUF_SIZE);

    if (rxCount == 0)
        return;

    // Send to ring
    if(rte_ring_sp_enqueue_bulk(ring, (void**) bufs, rxCount, NULL) == 0)
    {
        // Failed to send - packets are dropped
        for (int i = 0; i < rxCount; i++)
            rte_pktmbuf_free(bufs[i]);

        std::cout << "> Some packets [" << rxCount << "] were dropped!" << std::endl;
    }
    else
        std::cout << "> Packets sent: " << rxCount << std::endl;
}

void sendFromRingToEth(rte_ring *ring, int port)
{
    rte_mbuf *bufs[MBUF_SIZE];

    // Get packets from ring
    int rxCount = rte_ring_dequeue_burst(ring, (void**) bufs, MBUF_SIZE, NULL);

    if (rxCount == 0)
        return;

    std::cout << "> Received [" << rxCount << "] packets from [" << ring->name << "]";

    // Send goddamn packts
    int txCount = rte_eth_tx_burst(port, 0, bufs, rxCount);

    // Free mbufs that were not sent
    if (txCount != rxCount)
    {
        for (int i = txCount; i < rxCount; i++)
            rte_pktmbuf_free(bufs[i]);

        std::cout << " and DROPPED [" << rxCount - txCount << "] of them!" << std::endl;
    }
    else
        std::cout << " and sent all of them." << std::endl;
}

void sendFromRingToRing(rte_ring *rxRing, rte_ring *txRing)
{
    rte_mbuf *bufs[MBUF_SIZE];

    // Get packets from ring
    int rxCount = rte_ring_dequeue_burst(rxRing, (void**) bufs, MBUF_SIZE, NULL);

    if (rxCount == 0)
        return;

    std::cout << "> [" << rxCount << "] packets from " << rxRing->name;

    // Send to ring
    if(rte_ring_sp_enqueue_bulk(txRing, (void**) bufs, rxCount, NULL) == 0)
    {
        // Failed to send - packets are dropped
        for (int i = 0; i < rxCount; i++)
            rte_pktmbuf_free(bufs[i]);

        std::cout << " were dropped!" << std::endl;
    }
    else
        std::cout << " sent to " << txRing->name << std::endl;
}