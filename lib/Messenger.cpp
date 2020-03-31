#include <unistd.h>

#include "Messenger.h"

#include "Log.h"

Messenger::Messenger(const std::string& mempoolName,
                     const std::string& ringName)
{
    lookupRing(ringName);
    lookupMempool(mempoolName);
}

void Messenger::lookupMempool(const std::string& mempoolName)
{
    _mempool = rte_mempool_lookup(mempoolName.c_str());

    if(!_mempool)
        rte_exit(EXIT_FAILURE, "ERROR: Can't find mempool [%s]\n",
                                            mempoolName.c_str());
}

void Messenger::lookupRing(const std::string& ringName)
{
    _ring = rte_ring_lookup(ringName.c_str());

    if (!_ring)
        rte_exit(EXIT_FAILURE, "ERROR: Can't find ring [%s]\n",
                                            ringName.c_str());
}

void Messenger::sendMessage(const Header& mh, buff_type *data)
{
    uint8_t *memory = NULL;

    if (rte_mempool_get(_mempool, (void **) &memory) != 0)
        rte_panic("ERROR: Can't get message buffer\n");

    // COPY of packets - can be changed later
    memcpy(memory, &mh, sizeof(mh));
    memcpy(memory + sizeof(mh), data, mh.dataLength * sizeof(buff_type));

    // Send to ring
    if (rte_ring_mp_enqueue(_ring, memory) < 0) {
		Logl("Error: Failed to send message");

        // Put back
		rte_mempool_put(_mempool, memory);
	}
}

void Messenger::recvMessage(Header& mh, buff_type* data)
{
    uint8_t *memory = NULL;

    while (rte_ring_dequeue(_ring, (void **) &memory) != 0)
    {
        usleep(1000);
        continue;
    }

    // COPY of packets - can be changed later
    memcpy(&mh, memory, sizeof(mh));
    memcpy(data, memory + sizeof(mh), mh.dataLength * sizeof(buff_type));

    // Put back memory
    rte_mempool_put(_mempool, memory);
}