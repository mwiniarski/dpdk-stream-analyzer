#include "Ring.h"

#include "Log.h"

using namespace std;

const string Ring::PREFIX = "RING_";
const int Ring::SIZE = 128;

Ring::Ring(uint appIndex, uint chainIndex, bool create)
{
    string ringName = getName(chainIndex, appIndex);

    // Create ring
    if (create)
    {
        _ring = rte_ring_create(ringName.c_str(), SIZE, rte_socket_id(), 0);

        if (!_ring)
            rte_exit(EXIT_FAILURE, "ERROR: Can't create ring [%s]\n", ringName.c_str());

        Logl(">>> " << ringName << " created!");
    }
    else
    {
        _ring = rte_ring_lookup(ringName.c_str());

        if (!_ring)
            rte_exit(EXIT_FAILURE, "ERROR: Can't find ring [%s]\n", ringName.c_str());
    }
}

string Ring::getName(uint chainIndex, uint appIndex)
{
    return (PREFIX + to_string(chainIndex) + "_" + to_string(appIndex));
}

void Ring::getPackets(Buffer &buf)
{
    // Get packets from ring
    buf.size = rte_ring_dequeue_burst(_ring, (void**) buf.data, buf.CAPACITY, NULL);
}

void Ring::sendPackets(Buffer &buf)
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