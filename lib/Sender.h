#ifndef __SENDER__
#define __SENDER__

#include <memory>
#include <functional>

#include "Device.h"

class Sender
{
public:
    Sender(Device& rx,
           Device& tx,
           std::function<void (Packet &&p)> cb);

    void sendPacketBurst();

private:
    void startTimer();
    void measureTime();

private:
    Buffer _buffer;

    Device& _rxDevice;
    Device& _txDevice;
    std::function<void (Packet &&p)> _callback;
};

#endif