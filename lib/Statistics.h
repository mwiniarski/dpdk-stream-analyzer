#ifndef __STATISTICS__
#define __STATISTICS__

#include <cstdint>
#include <memory>
#include <chrono>
#include <string>

#include <InfluxDBFactory.h>

#include "GlobalInfo.h"
#include "Log.h"


class Statistics
{
public:
    enum Type { ETH = 0, APP = 1};

    Statistics(int type, int chainInd, int appInd);

    void addCycles(uint64_t cycles);
    void addBytes(uint64_t bytes);
    void addLinkCap(uint64_t linkCap);
    void addPackets(uint64_t packets);
    void addDropped(int dropped);

    void try_flush();    

private:
    using time_period = std::chrono::microseconds;

    constexpr int64_t one_sec()
    {
        return time_period(std::chrono::seconds(1)).count();
    };

    const int64_t getTime()
    {
        return std::chrono::duration_cast<time_period>(
                std::chrono::system_clock::now().time_since_epoch()).count();
    }

    // Accumulative counters
    struct Stats 
    {
        uint64_t dropped;
        uint64_t packets;
        uint64_t cycles;
        uint64_t bytes;
        uint64_t linkCap;
    } _stats = {};

    // Info
    const std::string _type;
    const std::string _chainIndex;
    const std::string _appIndex;

    // Initial values for timer and message ring
    std::chrono::system_clock::time_point _lastFlush;

    std::unique_ptr<influxdb::InfluxDB> _influxdb;
};

#endif