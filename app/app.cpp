#include <unistd.h>
#include <vector>

#include <rte_memzone.h>

#include <shared.h>

using namespace std;

GlobalInfo* info;

int newPacketCallback(Packet&& packet)
{
    return calcPacketHash(packet, info->packetWork);
}

int main(int argc, char* argv[])
{
    initEAL(argc, &argv);

    if (argc < 2)
        rte_exit(EXIT_FAILURE, "Usage: ./app <chain> <app_in_chain>\n");

    int chainIndex = stoi(argv[1]);
    int appIndex = stoi(argv[2]);

    // Retrieve global info
    info = GlobalInfo::get();

    // Retrieve rx ring used by app
    Ring rxRing(appIndex, chainIndex);

    unique_ptr<Sender> sender;

    // Use either ring or port for TX
    unique_ptr<Ring> txRing;
    unique_ptr<Port> txPort;

    // It is not the last app in chain - send to next ring
    if (!info->isLastInChain(appIndex, chainIndex))
    {
        Logl(">>> MIDDLE app mode");

        txRing = make_unique<Ring>(appIndex + 1, chainIndex);
        sender = make_unique<Sender>(rxRing, *txRing, newPacketCallback, true);
    }
    else
    // It is the last app - send to eth
    {
        Logl(">>> LAST-IN-CHAIN app mode");

        txPort = make_unique<Port>(info->txPort);
        txPort->setTxIndex(chainIndex);
        sender = make_unique<Sender>(rxRing, *txPort, newPacketCallback, true);
    }

    // Schedule
    schedule(SCHED_FIFO);

    // Main loop
    int counter = 0;
    auto now = chrono::system_clock::now();
    auto now2 = chrono::system_clock::now();

    for (;;)
    {
        counter++;

        // RING --> ETH
        const int c = sender->sendPacketBurst();

        if(c < MBuffer::CAPACITY || counter == info->loopsBeforeSwitch)
        {
            sender->addSwitch(counter);
            auto now2Old = now2;
            now = chrono::system_clock::now();
            sched_yield();
            now2 = chrono::system_clock::now();

            auto workTime = now - now2Old;
            auto switchTime = (now2 - now - workTime * (info->chainCount - 1)) / info->chainCount;

            sender->addTimes(switchTime.count(), workTime.count());
            counter = 0;
        }
    }
}