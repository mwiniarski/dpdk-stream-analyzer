#include <iostream>
#include <unistd.h>

#include <rte_memzone.h>

#include <common/common.h>

using namespace std;

// === GLOBALS
#define RX_RING_NAME RING_NAME_1
#define TX_RING_NAME RING_NAME_2

// Structure stored in memory shared between processes
PortInfo *portInfo;

// Ring to receive from
rte_ring  *rxRing;

//Ring to send back to
rte_ring *txRing;
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

void initRings()
{
    // RX ring
    rxRing = rte_ring_lookup(RX_RING_NAME);

    if (!rxRing)
        rte_exit(EXIT_FAILURE, "ERROR: Can't find RX ring '%s'", RX_RING_NAME);

    // TX ring
    txRing = rte_ring_lookup(TX_RING_NAME);

    if (!txRing)
        rte_exit(EXIT_FAILURE, "ERROR: Can't find TX ring '%s'", TX_RING_NAME);
}

void forwardPackets()
{
    rte_mbuf *bufs[MBUF_SIZE];

    // Get packets from ring
    int rxCount = rte_ring_dequeue_burst(rxRing, (void**) bufs, MBUF_SIZE, NULL);

    if (rxCount == 0)
        return;

    cout << "> Received [" << rxCount << "] packets from [" << RX_RING_NAME << "]";

    // Send goddamn packts
    int txCount = rte_eth_tx_burst(portInfo->txID, 0, bufs, rxCount);

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

void backwardPackets()
{
    rte_mbuf *bufs[MBUF_SIZE];

    // Get packets from eth
    int rxCount = rte_eth_rx_burst(portInfo->txID, 0, bufs, MBUF_SIZE);

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

int main(int argc, char* argv[])
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