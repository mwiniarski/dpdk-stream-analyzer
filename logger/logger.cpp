#include <unistd.h>
#include <vector>

#include <rte_memzone.h>

#include <shared.h>

using namespace std;

double getMicro(uint64_t delta)
{
    return (double) delta * 1000000.0L / (double) rte_get_timer_hz();
}

int main(int argc, char* argv[])
{
    initEAL(argc, &argv);

    // Retrieve global info
    GlobalInfo* info = GlobalInfo::get();

    // Get the messenger to receive packets
    Messenger messenger(info->MEMPOOL, info->STATS_RING);

    Messenger::Header mh;
    Messenger::buff_type buffer[Messenger::BUFFER_MAX];

    for (;;)
    {
        messenger.recvMessage(mh, buffer);

        string title = (mh.reporter == mh.APP ? "APP" : "ETH");

        // Mean
        int mean = 0;
        for (int i = 0; i < mh.dataLength; i++)
        {
            mean += buffer[i];
        }
        mean /= mh.dataLength;

        Logl(title << " [" << mh.chainIndex << ", " << mh.appIndex << "] - " << mh.dataLength << " packets"
                   << ", mean = " << getMicro(mean) << "us");
    }
}