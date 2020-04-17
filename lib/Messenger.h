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
    struct Data
    {
        uint64_t cycles;
        uint64_t bytes;
        uint32_t linkCap;
    };

    struct Header
    {
        enum Type { ETH = 0, APP = 1};

        Type reporter;
        int chainIndex;
        int appIndex;
        int dataLength;   // Count of elements stored in buffer
        int64_t timestamp;
    };

    static const int BUFFER_MAX = (RTE_MBUF_DEFAULT_BUF_SIZE - sizeof(Header)) / sizeof(Data);
    static const int PACKETS_PER_DATA_POINT = 1024;

public:
    /**
     * Access ring and mempool specified by parameters. Both mempool
     * and ring must exist before.
     *
     * @param mempoolName   Name of the mempool
     * @param ringName      Name of the ring
     */
    Messenger(const std::string& mempoolName,
              const std::string& ringName);

    /**
     * Send a message via ring communication.
     *
     * @param header    Header with at least correctly filled dataLenght
     * @param data      Buffer of max size BUFFER_MAX * sizeof(buff_type)
     */
    void sendMessage(const Header& header, Data* data);

    /**
     * Receive a message via ring communication and copy it
     * into header and data.
     *
     * @param header    Empty header. Received header will be copied into it
     * @param data      Allocated data of size BUFFER_MAX * sizeof(buff_type)
     */
    void recvMessage(Header& header, Data* data);

private:
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