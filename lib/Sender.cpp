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

int Sender::sendPacketBurst()
{
    _rxDevice.getPackets(_buffer);

    for (uint i = 0; i < _buffer.size; i++)
    {
        _callback(Packet(_buffer[i]));
    }

    collectStats();
    _stats->addDropped(_txDevice.sendPackets(_buffer));
    _stats->try_flush();

    return _buffer.size;
}

void Sender::collectStats()
{  
    uint64_t now = rte_rdtsc();

    // Add remaining packets to counter
    for (uint i = 0; i < _buffer.size; i++)
    {
        _stats->addCycles(now - _buffer.data[i]->udata64);
        _stats->addBytes(_buffer.data[i]->pkt_len);
    }

    _stats->addPackets(_buffer.size);

    // Update the number of buffers used
    _stats->addLinkCap(_buffer.CAPACITY);
}