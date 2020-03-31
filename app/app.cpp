#include <unistd.h>
#include <vector>

#include <rte_memzone.h>

#include <shared.h>

using namespace std;

void newPacketCallback(Packet &&packet)
{
    static int c = 1;

    if (c++ % 1000 == 0)
    {
        Logl("App " << c);
    }
}

int main(int argc, char* argv[])
{
    initEAL(argc, &argv);

    if (argc < 2)
        rte_exit(EXIT_FAILURE, "Usage: ./app <chain> <app_in_chain>\n");

    int chainIndex = stoi(argv[1]);
    int appIndex = stoi(argv[2]);

    // Retrieve global info
    GlobalInfo* info = GlobalInfo::get();

    // Retrieve rx ring used by app
    Ring rxRing(appIndex, chainIndex);

    // It is not the last app in chain - send to next ring
    if (!info->isLastInChain(appIndex, chainIndex))
    {
        Logl(">>> MIDDLE app mode");

        Ring txRing(appIndex + 1, chainIndex);
        Sender ringToRing(rxRing, txRing, newPacketCallback);

        for (;;)
        {
            // Sleep 1ms
            usleep(1000);

            // RING --> RING
            ringToRing.sendPacketBurst();
        }
    }
    else
    // It is the last app - send to eth
    {
        Logl(">>> LAST-IN-CHAIN app mode");

        Port txPort(info->txPort);
        Sender ringToEth(rxRing, txPort, newPacketCallback);

        for (;;)
        {
            // RING --> ETH
            ringToEth.sendPacketBurst();

            usleep(1000);
        }
    }

}