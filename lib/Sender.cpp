#include "Sender.h"

using namespace std;
using namespace std::chrono;

Sender::Sender(Device& rx,
               Device& tx,
               function<int (Packet &&p)> cb,
               bool collectStats)
:_rxDevice(rx),
 _txDevice(tx),
 _callback(cb),
 _collectStats(collectStats)
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
    auto t1 = system_clock::now();
    int sum = 0;
    // Do the work
    for (uint i = 0; i < _buffer.size; i++)
    {
        sum += _callback(Packet(_buffer[i]));
    }

    // Collect stats as for NOW
    if(_collectStats)
    {
        collectStats(t1.time_since_epoch().count());
    }

    int64_t now = system_clock::now().time_since_epoch().count();
    
    // Update send-moment as for NOW
    for (uint i = 0; i < _buffer.size; i++)
    {
        _buffer.data[i]->udata64 = now;
    }

    // Send packets
    int dropped = _txDevice.sendPackets(_buffer);

    if(_collectStats)
    {
        _stats->addDropped(dropped);
        _stats->try_flush();
    }

    return _buffer.size;
}

void Sender::collectStats(int64_t now)
{  
    // Add remaining packets to counter
    for (uint i = 0; i < _buffer.size; i++)
    {
        _stats->addCycles(now - (int64_t)_buffer.data[i]->udata64);
        _stats->addBytes(_buffer.data[i]->pkt_len);
    }

    _stats->addPackets(_buffer.size);

    // Update the number of buffers used
    _stats->addLinkCap(_buffer.CAPACITY);
}

void Sender::addSwitch(int count)
{
    _stats->addSwitch(count);
}

void Sender::addTimes(int64_t switchTime, int64_t workTime)
{
    _stats->addSwitchTime(switchTime);
    _stats->addWorkTime(workTime);
}