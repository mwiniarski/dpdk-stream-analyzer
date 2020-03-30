#include "Sender.h"

using namespace std;

Sender::Sender(Device& rx,
               Device& tx,
               function<void (Packet &&p)> cb)
:_rxDevice(rx),
 _txDevice(tx),
 _callback(cb)
{
    // Decide which Device has chain information
    bool txDevType = _txDevice.isApp();
    int chainInd = (txDevType ? _txDevice : _rxDevice).getPosition().first;
    int appInd = _txDevice.getPosition().second;

    _stats = make_unique<Statistics>(txDevType, chainInd, appInd);
}

void Sender::sendPacketBurst()
{
    _rxDevice.getPackets(_buffer);

    for (int i = 0; i < _buffer.size; i++)
    {
        _callback(Packet(_buffer[i]));
    }

    collectStats();
    _txDevice.sendPackets(_buffer);
}


void Sender::collectStats()
{
    int i = 0;
    uint64_t now = rte_rdtsc();

    // Calculate number of packets after flush
    int overflow = (_stats->packets + _buffer.size) - _stats->PACKETS_MAX;

    // If packets counter will overflow we need to fill it till MAX and flush
    if (overflow > 0)
    {
        // Add remaining packets to counter
        for (; i < _buffer.size - overflow; i++)
            _stats->cycles += (now - _buffer.data[i]->udata64);

        // Move packets from counter to buffer of averages
        _stats->flush();
    }

    // Add remaining packets to counter
    for (; i < _buffer.size; i++)
        _stats->cycles += (now - _buffer.data[i]->udata64);

    _stats->packets += _buffer.size;
}