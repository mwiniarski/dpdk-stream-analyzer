#ifndef __STATISTICS__
#define __STATISTICS__

#include <cstdint>
#include <memory>
#include <chrono>

#include "Messenger.h"
#include "GlobalInfo.h"
#include "Log.h"

class Statistics
{
public:
    enum Type { ETH = 0, APP = 1 };

    Statistics(int type, int chainInd, int appInd)
        :_type((Type) type),
         _chainIndex(chainInd),
         _appIndex(appInd)
    {}

    static const int PACKETS_MAX = Messenger::PACKETS_PER_DATA_POINT;

    // Temporary counters
    int packets = 0;
    Messenger::Data data = {};

    // Move from temporary to accumulative
    inline void flush()
    {
        int64_t now = getTime();

        _lastFlush = now;

        // Add to buffer
        _buffer[_size++] = data;
        data = {};
        packets = 0;

        // 1 second passed or buffer full
        if (now - _lastSent > one_sec() ||
            _size == BUFFER_MAX)
        {
            _lastSent = now;
            send();
        }
    }

private:
    // Send accumulated data to server and clear
    inline void send()
    {
        // Send
        Messenger::Header h =
        {
            .reporter = (Messenger::Header::Type) _type,
            .chainIndex = _chainIndex,
            .appIndex = _appIndex,
            .dataLength = _size,
            .timestamp = _lastSent
        };

        _messenger.sendMessage(h, _buffer);

        _size = 0;
    }

    using time_period = std::chrono::microseconds;

    constexpr int64_t one_sec()
    {
        return time_period(std::chrono::seconds(1)).count();
    };

    const int64_t getTime()
    {
        return std::chrono::duration_cast<time_period>(
                   std::chrono::system_clock::now().time_since_epoch()
               ).count();
    }

    // Accumulative counter
    static const int BUFFER_MAX = Messenger::BUFFER_MAX;
    Messenger::Data _buffer[BUFFER_MAX];
    int _size = 0;

    // Info
    Type _type;
    int _chainIndex;
    int _appIndex;

    // Initial values for timer and message ring
    int64_t _lastSent {getTime()};
    int64_t _lastFlush {getTime()};
    Messenger _messenger {GlobalInfo::MEMPOOL, GlobalInfo::STATS_RING};
};

#endif