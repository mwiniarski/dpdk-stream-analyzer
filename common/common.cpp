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

void sendFromEthToEth(int rxPort, int txPort)
{
    rte_mbuf *bufs[MBUF_SIZE];

    // Get packets from eth
    int rxCount = rte_eth_rx_burst(rxPort, 0, bufs, MBUF_SIZE);

    if (rxCount == 0)
        return;

    // Send goddamn packts
    int txCount = rte_eth_tx_burst(txPort, 0, bufs, rxCount);

    // Free mbufs that were not sent
    if (txCount != rxCount)
    {
        for (int i = txCount; i < rxCount; i++)
            rte_pktmbuf_free(bufs[i]);

        Log(">> DROP! >> ");
    }

    Logl("Eth" << rxPort << " [" << rxCount << "] -> Eth"
               << txPort << " [" << txCount << "]");
}

void sendFromEthToRing(int port, rte_ring *ring)
{
    rte_mbuf *bufs[MBUF_SIZE];

    // Get packets from eth
    int rxCount = rte_eth_rx_burst(port, 0, bufs, MBUF_SIZE);

    if (rxCount == 0)
        return;

    // Send to ring
    int txCount = rte_ring_sp_enqueue_bulk(ring, (void**) bufs, rxCount, NULL);

    if (rxCount != txCount)
    {
        // Failed to send - packets are dropped
        for (int i = 0; i < rxCount; i++)
            rte_pktmbuf_free(bufs[i]);

        Log(">> DROP! >> ");
    }

    Logl("Eth" << port << " [" << rxCount << "] -> "         << ring->name << " [" << txCount << "]");
}

void sendFromRingToEth(rte_ring *ring, int port)
{
    rte_mbuf *bufs[MBUF_SIZE];

    // Get packets from ring
    int rxCount = rte_ring_dequeue_burst(ring, (void**) bufs, MBUF_SIZE, NULL);

    if (rxCount == 0)
        return;

    // Send goddamn packts
    int txCount = rte_eth_tx_burst(port, 0, bufs, rxCount);

    // Free mbufs that were not sent
    if (txCount != rxCount)
    {
        for (int i = txCount; i < rxCount; i++)
            rte_pktmbuf_free(bufs[i]);

        Log(">> DROP! >> ");
    }

    Logl(ring->name << " [" << rxCount << "] -> Eth"
            << port << " [" << txCount << "]");
}

void sendFromRingToRing(rte_ring *rxRing, rte_ring *txRing)
{
    rte_mbuf *bufs[MBUF_SIZE];

    // Get packets from ring
    int rxCount = rte_ring_dequeue_burst(rxRing, (void**) bufs, MBUF_SIZE, NULL);

    if (rxCount == 0)
        return;

    // Send to ring
    int txCount = rte_ring_sp_enqueue_bulk(txRing, (void**) bufs, rxCount, NULL);

    if (txCount != rxCount)
    {
        // Failed to send - packets are dropped
        for (int i = 0; i < rxCount; i++)
            rte_pktmbuf_free(bufs[i]);

        Log(">> DROP! >> ");
    }

    Logl(rxRing->name << " [" << rxCount << "] -> "
      << txRing->name << " [" << txCount << "]");
}

