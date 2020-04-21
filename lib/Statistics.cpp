#include "Statistics.h"

using namespace std;
using namespace std::chrono;

Statistics::Statistics(int type, int chainInd, int appInd)
    :_type(type == ETH ? "ETH" : "APP"),
    _chainIndex(to_string(chainInd)),
    _appIndex(to_string(appInd)),
    _lastFlush(system_clock::now())
{
    _influxdb = influxdb::InfluxDBFactory::Get("http://localhost:8086/?db=mydb");
}

// Move from temporary to accumulative
void Statistics::try_flush()
{
    auto now = system_clock::now();

    // 1 second passed
    if (now - _lastFlush > 1s && _stats.packets != 0)
    {
        double secs = (double) duration_cast<microseconds>(now - _lastFlush).count() / 1000000.0L;
        _lastFlush = now;

        double throughput = (double) _stats.bytes / secs;
        double link = 100.0L * _stats.packets / _stats.linkCap;
        double dropped = 100.0f * _stats.dropped / _stats.packets;
        double latency = 1000000.0L * _stats.cycles / rte_get_timer_hz() / _stats.packets;

        _influxdb->write(influxdb::Point{GlobalInfo::STATS_NAME}
            .addTag("type", _type)
            .addTag("chain", _chainIndex)
            .addTag("index", _appIndex)
            .addField("throughput", throughput)
            .addField("latency", latency)
            .addField("link", link)
            .addField("dropped", dropped)
            .setTimestamp(now));

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

