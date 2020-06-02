#include "Statistics.h"

using namespace std;
using namespace std::chrono;

Statistics::Statistics(int type, int chainInd, int appInd)
    :_type(type),
    _chainIndex(chainInd),
    _appIndex(appInd),
    _lastFlush(system_clock::now())
{
    _info = GlobalInfo::get();
}

// Move from temporary to accumulative
void Statistics::try_flush()
{
    auto now = system_clock::now();

    // 1 second passed
    if (now - _lastFlush > STAT_DELAY && _stats.packets != 0)
    {
        double secs = (double) duration_cast<microseconds>(now - _lastFlush).count() / 1000000.0L;
        _lastFlush = now;

        double throughput = (double) _stats.bytes * 8 / secs;
        double link = 100.0L * _stats.packets / _stats.linkCap;
        double dropped = 100.0L * _stats.dropped / _stats.packets;
        double latency = _stats.cycles / _stats.packets / 1000.0L;
        double procSpeed = (double)_stats.bytes / _stats.workTime * 8 * 1000000000.0L; 

        double workToSwitch = 0, switchTime = 0, workTime = 0;
        if (_stats.switches)
        {
            workTime = _stats.workTime / _stats.switches / 1000.0L;
            workToSwitch = (double)_stats.loopsBeforeSwitch / _stats.switches;
            switchTime = (double)_stats.switchTime / _stats.switches / 1000.0L;
        }

        double theoreticalLatency = 0.5 * switchTime * (procSpeed - throughput) / (procSpeed / _info->chainCount - throughput);
        
        Messenger::Header mh =
        {
            .type = _type,
            .chainIndex = _chainIndex,
            .appIndex = _appIndex,
            .throughput = throughput,
            .latency = latency,
            .theoreticalLatency = theoreticalLatency,
            .workTime = workTime,
            .workToSwitch = workToSwitch,
            .switchTime = switchTime,
            .procSpeed = procSpeed,
            .link = link,
            .dropped = dropped,
            .timestamp = now.time_since_epoch().count(),
        };

        _messenger.sendMessage(mh);

        _stats = {};
    }
}

void Statistics::addCycles(int64_t cycles)
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

void Statistics::addWorkTime(int64_t workTime)
{
    _stats.workTime += workTime;
}

void Statistics::addSwitch(int count)
{
    _stats.switches++;
    _stats.loopsBeforeSwitch += count;
}

void Statistics::addSwitchTime(int64_t switchTime)
{
    _stats.switchTime += switchTime;
}
