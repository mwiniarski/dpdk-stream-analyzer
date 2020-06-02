#include "GlobalInfo.h"
#include "Ring.h"
#include "Log.h"

#include <chrono>
using namespace std;
using namespace std::chrono;

const string Ring::PREFIX = "RING_";
const int Ring::SIZE = 512;

Ring::Ring(int appInd, int chainInd, bool createNew)
    :Device(chainInd, appInd)
{
    string name = getName(_chainIndex, _appIndex);

    if (createNew)
        create(name);
    else
        lookup(name);
}

Ring::Ring(const std::string& name)
{
    create(name);
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

int Ring::size()
{
    return rte_ring_count(_ring);
}

void Ring::getPackets(MBuffer &buf)
{
    // Get packets from ring
    buf.size = rte_ring_sc_dequeue_burst(_ring, (void**) buf.data, buf.CAPACITY, NULL);
}

int Ring::sendPackets(MBuffer &buf)
{
    // Send to ring
    uint free_space;
    uint txCount = rte_ring_sp_enqueue_bulk(_ring, (void**) buf.data, buf.size, &free_space);

    // Free mbufs that were not sent
    if (txCount != buf.size)
    {
        for (uint i = 0; i < buf.size; i++)
            rte_pktmbuf_free(buf[i]);
    }

    return buf.size - txCount;
}