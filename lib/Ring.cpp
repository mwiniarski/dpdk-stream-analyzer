#include "GlobalInfo.h"
#include "Ring.h"
#include "Log.h"

using namespace std;

const string Ring::PREFIX = "RING_";
const int Ring::SIZE = 128;

Ring::Ring(int appInd, int chainInd, bool createNew)
    :Device(chainInd, appInd)
{
    init(getName(_chainIndex, _appIndex), createNew);
}

Ring::Ring(const std::string& name, bool createNew)
{
    init(name, createNew);

    // This constructor is used to communicate stats, hence mempool
    getMempool();
}

void Ring::init(const std::string& name, bool createNew)
{
    if (createNew)
        create(name);
    else
        lookup(name);
}

void Ring::getMempool()
{
    _mempool = rte_mempool_lookup(GlobalInfo::MEMPOOL.c_str());

    if(!_mempool)
        rte_exit(EXIT_FAILURE, "ERROR: Can't find mempool [%s]\n",
                                            GlobalInfo::MEMPOOL.c_str());
}

void Ring::create(const std::string& ringName)
{
    _ring = rte_ring_create(ringName.c_str(), SIZE, rte_socket_id(), 0);

    if (!_ring)
        rte_exit(EXIT_FAILURE, "ERROR: Can't create ring [%s]\n", ringName.c_str());

    Logl(">>> " << ringName << " created!");
}

void Ring::lookup(const std::string& ringName)
{
    _ring = rte_ring_lookup(ringName.c_str());

    if (!_ring)
        rte_exit(EXIT_FAILURE, "ERROR: Can't find ring [%s]\n", ringName.c_str());
}

string Ring::getName(int chainIndex, int appIndex)
{
    return (PREFIX + to_string(chainIndex) + "_" + to_string(appIndex));
}

void Ring::getPackets(MBuffer &buf)
{
    // Get packets from ring
    buf.size = rte_ring_dequeue_burst(_ring, (void**) buf.data, buf.CAPACITY, NULL);
}

void Ring::sendPackets(MBuffer &buf)
{
    // Send to ring
    int txCount = rte_ring_sp_enqueue_bulk(_ring, (void**) buf.data, buf.size, NULL);

    // Free mbufs that were not sent
    if (txCount != buf.size)
    {
        for (int i = 0; i < buf.size; i++)
            rte_pktmbuf_free(buf[i]);

        _dropped += buf.size;
    }
}

void Ring::sendMessage(const MessageHeader& mh)
{
    uint8_t *memory = NULL;

    if (rte_mempool_get(_mempool, (void **) &memory) != 0)
        rte_panic("ERROR: Can't get message buffer\n");

    // COPY of packets - can be changed later
    memcpy(memory, &mh, sizeof(mh));
    memcpy(memory + sizeof(mh), mh.data, mh.dataLength);

    // Send to ring
    if (rte_ring_mp_enqueue(_ring, memory) < 0) {
		Logl("Error: Failed to send message");

        // Put back
		rte_mempool_put(_mempool, memory);
	}
}