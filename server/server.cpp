#include <unistd.h>

#include <iostream>
#include <string>

#include "common.h"

// === GLOBALS
#define MBUF_COUNT 8192
#define MBUF_CACHE_SIZE 250
#define MBUF_POOL_NAME "MBUF_POOL"

#define ETH_RING_SIZE 512
#define BURST_SIZE 32

#define SEND_RING_NAME "RING_1"
#define SEND_RING_SIZE 128

// Structure stored in memory shared between processes
PortInfo *portInfo;

// Static memory to use by all aplications to pass packets
rte_mempool *mbufPool;

// First application ring
rte_ring *sendRing;

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

    memset(memZone->addr, 0, sizeof(PortInfo));
    portInfo = (PortInfo*) memZone->addr;

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
    sendRing = rte_ring_create(SEND_RING_NAME, SEND_RING_SIZE, rte_socket_id(), 0);

    if (!sendRing)
		rte_exit(EXIT_FAILURE, "ERROR: Problem with creating send ring\n"); 

    cout << ">>> Ring " << SEND_RING_NAME << " [" 
         << SEND_RING_SIZE << "] created" << endl;
}

void startForwarding()
{
    rte_mbuf *bufs[BURST_SIZE];

    for (;;)
    {
        // Get packets from eth
        const uint16_t rx_count = rte_eth_rx_burst(0, 0, bufs, BURST_SIZE);

        // Sleep 0.5 sec
        usleep(500000);

        if (rx_count == 0)
            continue;

        // Send to ring
        if(rte_ring_sp_enqueue_bulk(sendRing, (void**) bufs, rx_count, NULL) == 0)
        {
            // Failed to send - packets are dropped
            for (int i = 0; i < rx_count; i++)
                rte_pktmbuf_free(bufs[i]);

            cout << "> Some packets [" << rx_count << "] were dropped!" << endl;
        }
        else
            cout << "> Packets sent: " << rx_count << endl;
    }
}

int main(int argc, char *argv[])
{
    initEAL(argc, &argv);

    initEth();

    initRings();

    startForwarding();
}