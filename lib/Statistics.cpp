#include "Statistics.h"

using namespace std;
using namespace std::chrono;

Statistics::Statistics(int type, int chainInd, int appInd)
    :_type(type),
    _chainIndex(chainInd),
    _appIndex(appInd),
    _lastFlush(system_clock::now())
{}

// Move from temporary to accumulative
void Statistics::try_flush()
{
    auto now = system_clock::now();

    // 1 second passed
    if (now - _lastFlush > 1s && _stats.packets != 0)
    {
        double secs = (double) duration_cast<microseconds>(now - _lastFlush).count() / 1000000.0L;
        _lastFlush = now;

        uint64_t throughput = (double) _stats.bytes / secs;
        double link = 100.0L * _stats.packets / _stats.linkCap;
        double dropped = 100.0L * _stats.dropped / _stats.packets;
        double latency = 1000000.0L * _stats.cycles / rte_get_timer_hz() / _stats.packets;

        Messenger::Header mh =
        {
            .type = _type,
            .chainIndex = _chainIndex,
            .appIndex = _appIndex,
            .throughput = throughput,
            .latency = latency,
            .link = link,
            .dropped = dropped,
            .timestamp = now.time_since_epoch().count(),
        };

        _messenger.sendMessage(mh);

        _stats = {};
    }
}

void Statistics::addCycles(uint64_t cycles)
{
    _stats.cycles += cycles;
}

void Statistics::addBytes(uint64_t bytes)
{
    _stats.bytes += bytes;
}

void Statistics::addLinkCap(uint64_t linkCap)
{
    _stats.linkCap += linkCap;
}

void Statistics::addPackets(uint64_t packets)
{
    _stats.packets += packets;
}

void Statistics::addDropped(int dropped)
{
    _stats.dropped += dropped;
}

