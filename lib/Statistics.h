#ifndef __STATISTICS__
#define __STATISTICS__

#include <cstdint>
#include <memory>
#include <chrono>
#include <string>

#include "GlobalInfo.h"
#include "Messenger.h"
#include "Log.h"


class Statistics
{
public:
    Statistics(int type, int chainInd, int appInd);

    void addCycles(int64_t cycles);
    void addBytes(uint64_t bytes);
    void addLinkCap(uint64_t linkCap);
    void addPackets(uint64_t packets);
    void addWorkTime(int64_t workTime);
    void addDropped(int dropped);
    void addSwitch(int count);
    void addSwitchTime(int64_t switchTime);

    void try_flush();    

private:
    static constexpr auto STAT_DELAY = std::chrono::seconds(1);

    // Accumulative counters
    struct Stats 
    {
        uint64_t dropped;
        uint64_t packets;
        uint64_t cycles;
        uint64_t bytes;
        uint64_t linkCap;
        uint64_t workTime;
        uint64_t switches;
        uint64_t switchTime;
        uint64_t loopsBeforeSwitch;
    } _stats = {};

    // Info
    const int _type;
    const int _chainIndex;
    const int _appIndex;

    // Initial values for timer and message ring
    std::chrono::system_clock::time_point _lastFlush;

    Messenger _messenger {GlobalInfo::STATS_MEMPOOL, GlobalInfo::STATS_RING};

    GlobalInfo *_info;
};

#endif