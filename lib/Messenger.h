#ifndef __MESSENGER__
#define __MESSENGER__

#include <string>
#include <memory>

#include <rte_ring.h>
#include <rte_mbuf.h>
#include <rte_mempool.h>

class Messenger
{
public:
    enum Type { ETH = 0, APP = 1};

    struct Header
    {
        int type;
        int chainIndex;
        int appIndex;
        double throughput;
        double latency;
        double theoreticalLatency;
        double workTime;
        double workToSwitch;
        double switchTime;
        double procSpeed;
        double link;
        double dropped;
        int64_t timestamp;
    };

public:
    /**
     * Access ring and mempool specified by parameters. Both mempool
     * and ring must exist before.
     *
     * @param mempoolName   Name of the mempool
     * @param ringName      Name of the ring
     */
    Messenger(const std::string& mempoolName,
              const std::string& ringName,
              bool createNew = false);

    /**
     * Send a message via ring communication.
     *
     * @param header    Header with at least correctly filled dataLenght
     * @param data      Buffer of max size BUFFER_MAX * sizeof(buff_type)
     */
    void sendMessage(const Header& header);

    /**
     * Receive a message via ring communication and copy it
     * into header and data.
     *
     * @param header    Empty header. Received header will be copied into it
     * @param data      Allocated data of size BUFFER_MAX * sizeof(buff_type)
     */
    void recvMessage(Header& header);

private:
    void createRing(const std::string& ringName);
    void createMempool(const std::string& mempoolName);

    // Find existing mempool by name
    void lookupMempool(const std::string& mempoolName);

    // Find existing ring by name
    void lookupRing(const std::string& ringName);

    // Pointer to underlying ring structure
    rte_ring *_ring;

    // Pointer to mempool used to send messages
    rte_mempool *_mempool;
};

#endif