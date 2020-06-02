#include <unistd.h>
#include <vector>
#include <iomanip>
#include <chrono>
#include <thread>

#include <rte_memzone.h>
#include <shared.h>

#include <InfluxDBFactory.h>

#include "CpuStats.h"

using namespace std;
using namespace std::chrono;

//#define LOOP_EXPERIMENT

double getMicro(uint64_t delta)
{
    return (double) delta * 1000000.0L / (double) rte_get_timer_hz();
}

int main(int argc, char* argv[])
{
    string url = "http://localhost:8086/?db=mydb";
    auto influxdb = influxdb::InfluxDBFactory::Get(url);

    initEAL(argc, &argv);

    string seriesName;
    if (argc > 1)
        seriesName = string(argv[1]);

    // Retrieve global info
    GlobalInfo* info = GlobalInfo::get();
    info->packetWork = stoi(argv[2]);

    Logl(">>> Packet workload: " << info->packetWork);

    // Get the messenger to receive packets
    Messenger messenger(info->STATS_MEMPOOL, info->STATS_RING);

    Messenger::Header mh;

    // Run CPU measurements in separate thread
    thread(&runCPUMeasurements, ref(url)).detach();

    for (;;)
    {
        messenger.recvMessage(mh);

        string title = (mh.type == Messenger::APP ? "APP" : "ETH");
        time_point<system_clock> tp(nanoseconds(mh.timestamp));

        // /*
        // Logl(title << " [" << mh.chainIndex << ", " << mh.appIndex << "] - " << setprecision(3)
        //            << "lat = " << mh.latency << "us "
        //            << "link = " << mh.link << "% "
        //            << "speed = " << mh.throughput / 1024 << "kB/s");
        // */

        // SELECT mean("throughput") as V1, mean("procSpeed") as V2, mean("workToSwitch") as R, mean("worktime") as T, mean("switchTime") as Ts, mean("latency") as Tw, mean("theoreticalLatency") as TTw FROM "stats2" WHERE ("type" = 'ETH' AND "index" = '1') and time > now() - 60s
        // SELECT mean("user") as cpu2 FROM CPUstats WHERE "cpu"='2' and time > now() - 60s
        Log("|" << flush);

        if (!seriesName.empty())
        {
            influxdb->write(influxdb::Point{seriesName}
                .addTag("type", title)
                .addTag("chain", to_string(mh.chainIndex))
                .addTag("index", to_string(mh.appIndex))
                .addField("throughput", mh.throughput)
                .addField("latency", mh.latency)
                .addField("theoreticalLatency", mh.theoreticalLatency)
                .addField("worktime", mh.workTime)
                .addField("workToSwitch", mh.workToSwitch)
                .addField("switchTime", mh.switchTime)
                .addField("procSpeed", mh.procSpeed)
                .addField("link", mh.link)
                .addField("dropped", mh.dropped)
                .setTimestamp(tp));
        }
    }
}