#ifndef __STATISTICS__
#define __STATISTICS__

#include <cstdint>
#include <memory>

#include <rte_cycles.h>

#include "Ring.h"
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

        Logl((_type ? "APP[" : "ETH[")
                << _chainIndex << ", "
                << _appIndex << "]: "
                << ((double) _buffer[_size - 1] / (double) rte_get_timer_hz()) * 1000000 << "us");

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
        Ring::MessageHeader mh;
        mh.dataLength = _size * sizeof(uint64_t);
        mh.reporter = Ring::MessageHeader::ETH;
        mh.appIndex = 1;
        mh.chainIndex = 1;
        mh.data = (uint8_t*) _buffer;

        _statsRing.sendMessage(mh);
        // ====

        _size = 0;
    }


    // Accumulative counter
    static const int BUFFER_MAX =
            RTE_MBUF_DEFAULT_DATAROOM / sizeof(uint64_t);

    uint64_t _buffer[BUFFER_MAX];
    int _size = 0;

    // Info
    Type _type;
    int _chainIndex;
    int _appIndex;

    // Initial values for timer and message ring
    uint64_t _lastSent {rte_rdtsc()};
    Ring _statsRing {GlobalInfo::STATS_RING};
};

#endif