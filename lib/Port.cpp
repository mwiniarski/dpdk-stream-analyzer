#include "Port.h"

Port::Port(int p, rte_mempool* mp)
    :_port(p)
{
    if (mp)
        init(mp);
}

void Port::getPackets(Buffer &buf)
{
    // Get packets from eth
    buf.size = rte_eth_rx_burst(_port, 0, buf.data, buf.CAPACITY);
}

void Port::sendPackets(Buffer &buf)
{
    // Send packets to eth
    int txCount = rte_eth_tx_burst(_port, 0, buf.data, buf.size);

    // Free mbufs that were not sent
    if (txCount != buf.size)
    {
        for (int i = txCount; i < buf.size; i++)
            rte_pktmbuf_free(buf[i]);

        _dropped += buf.size - txCount;
    }
}

int Port::init(rte_mempool *mbufPool)
{
    int ret;
    uint16_t nb_rxd = ETH_RING_SIZE;
    uint16_t nb_txd = ETH_RING_SIZE;

    // Configure eth device
    rte_eth_conf portConf;
    memset(&portConf, 0, sizeof(rte_eth_conf));
    portConf.rxmode.max_rx_pkt_len = ETHER_MAX_LEN;

    // Set 1 rx and 1 tx queue for given port
    ret = rte_eth_dev_configure(_port, 1, 1, &portConf);
    if (ret != 0)
        return ret;

    // Check if descriptor numbers are correct
    ret = rte_eth_dev_adjust_nb_rx_tx_desc(_port, &nb_rxd, &nb_txd);
    if (ret != 0)
        return ret;

    int sockId = rte_eth_dev_socket_id(_port);

    // Setup rx queue
    ret = rte_eth_rx_queue_setup(_port, 0, nb_rxd, sockId, NULL, mbufPool);
    if (ret < 0)
        return ret;

    // Setup tx queue
    ret = rte_eth_tx_queue_setup(_port, 0, nb_txd, sockId, NULL);
    if (ret < 0)
        return ret;

    // Start eth device
    ret = rte_eth_dev_start(_port);
    if (ret < 0)
        return ret;

    // Enable rx in promiscuous mode for eth device.
    rte_eth_promiscuous_enable(_port);

    return 0;
}