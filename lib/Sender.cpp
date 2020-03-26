#include "Sender.h"

using namespace std;

Sender::Sender(Device& rx,
               Device& tx,
               function<void (Packet &&p)> cb)
:_rxDevice(rx),
 _txDevice(tx),
 _callback(cb)
{}

void Sender::startTimer() {}
void Sender::measureTime() {}

void Sender::sendPacketBurst()
{
    _rxDevice.getPackets(_buffer);

    for (int i = 0; i < _buffer.size; i++)
    {
        _callback(Packet(_buffer[i]));
    }

    _txDevice.sendPackets(_buffer);
}
