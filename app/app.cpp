#include <unistd.h>
#include <vector>

#include <rte_memzone.h>

#include <shared.h>

using namespace std;

void newPacketCallback(Packet &&packet)
{
    static int c = 0;
    Logl("App " << c++);
}

int main(int argc, char* argv[])
{
    initEAL(argc, &argv);

    if (argc < 2)
        rte_exit(EXIT_FAILURE, "Usage: ./app <index>\n");

    // Retrieve global info
    GlobalInfo* info = GlobalInfo::get();

    // Retrieve rx ring used by app
    int appIndex = stoi(argv[1]);
    Ring rxRing(appIndex);

    // It is not the last app in chain - send to next ring
    if (appIndex + 1 < info->appCount)
    {
        Logl(">>> MIDDLE app mode");

        Ring txRing(appIndex + 1);
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
            // Sleep 1ms
            usleep(1000);

            // RING --> ETH
            ringToEth.sendPacketBurst();
        }
    }

}