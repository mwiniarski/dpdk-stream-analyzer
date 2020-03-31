#ifndef __STATISTICS__
#define __STATISTICS__

#include <cstdint>
#include <memory>

#include <rte_cycles.h>

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

    // Temporary counters
    static const int PACKETS_MAX = 1024;
    uint64_t cycles = 0;
    int packets = 0;

    // Move from temporary to accumulative
    inline void flush()
    {
        _buffer[_size++] = cycles / PACKETS_MAX;
        cycles = packets = 0;

        // 1 second passed or buffer full
        if (rte_rdtsc() - _lastSent > rte_get_timer_hz() ||
            _size == BUFFER_MAX)
        {
            send();
        }
    }

private:
    // Send accumulated data to server and clear
    inline void send()
    {
        _lastSent = rte_rdtsc();

        // SEND  (TODO)
        Messenger::Header h =
        {
            .reporter = (Messenger::Header::Type) _type,
            .chainIndex = _chainIndex,
            .appIndex = _appIndex,
            .dataLength = _size
        };

        _messenger.sendMessage(h, _buffer);
        // ====

        _size = 0;
    }

    // Accumulative counter
    static const int BUFFER_MAX = Messenger::BUFFER_MAX;
    uint64_t _buffer[BUFFER_MAX];
    int _size = 0;

    // Info
    Type _type;
    int _chainIndex;
    int _appIndex;

    // Initial values for timer and message ring
    uint64_t _lastSent {rte_rdtsc()};
    Messenger _messenger {GlobalInfo::MEMPOOL, GlobalInfo::STATS_RING};
};

#endif