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

void runCPUMeasurements(string& url)
{
    auto influxdb = influxdb::InfluxDBFactory::Get(url);

    auto oldStats = readStatsCPU();
    influxdb->batchOf(oldStats.size());

#ifdef LOOP_EXPERIMENT
    GlobalInfo* info = GlobalInfo::get();
    auto influxdb2 = influxdb::InfluxDBFactory::Get(url);
    auto currTime = system_clock::now();
    influxdb2->write(influxdb::Point{"LoopsAndWork"}
        .addField("loops", info->loopsBeforeSwitch)
        .addField("work", info->packetWork)
        .setTimestamp(currTime));

    uint stage = 1;
#endif
    for (;;)
    {
        sleep(1);

        auto newStats = readStatsCPU();
        auto diffs = calculateUsage(oldStats, newStats);
        auto now = system_clock::now();
        oldStats = newStats;

        for (uint i = 0; i < diffs.size(); i++)
        {
            influxdb->write(influxdb::Point{"CPUstats"}
                .addTag("cpu", to_string(i))
                .addField("user", diffs[i].user)
                .addField("system", diffs[i].system)
                .addField("idle", diffs[i].idle)
                .setTimestamp(now));
        }

#ifdef LOOP_EXPERIMENT
        if (now - currTime > 1.5s)
        {
            if (info->loopsBeforeSwitch == 5)
                break;

            info->loopsBeforeSwitch -= 5;
            stage++;
            currTime = now;
            Logl("Loops: " << info->loopsBeforeSwitch);

            influxdb2->write(influxdb::Point{"LoopsAndWork"}
                .addField("loops", info->loopsBeforeSwitch)
                .addField("work", info->packetWork)
                .setTimestamp(now));
        }
#endif
    }
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

    // Get the messenger to receive packets
    Messenger messenger(info->MEMPOOL, info->STATS_RING);

    Messenger::Header mh;
    Messenger::Data buffer[Messenger::BUFFER_MAX];

    time_point<system_clock> timePointsApp[10][10] = {system_clock::now()};
    time_point<system_clock> timePointsEth[11][2]  = {system_clock::now()};

    // Run CPU measurements in separate thread
    thread(&runCPUMeasurements, ref(url)).detach();

    for (;;)
    {
        messenger.recvMessage(mh, buffer);

        string title = (mh.reporter == mh.APP ? "APP" : "ETH");

        time_point<system_clock> tp(microseconds(mh.timestamp));

        system_clock::time_point *ltp;

        if (mh.reporter == mh.ETH)
        {
            ltp = &timePointsEth[mh.chainIndex + 1][mh.appIndex];
        }
        else
        {
            ltp = &timePointsApp[mh.chainIndex][mh.appIndex];
        }

        int64_t delta = duration_cast<microseconds>(tp - *ltp).count();
        *ltp = tp;

        // Means
        uint64_t meanLatency = 0;
        uint64_t linkCap = 0;
        uint64_t bytes = 0;

        for (int i = 0; i < mh.dataLength; i++)
        {
            meanLatency += buffer[i].cycles;
            linkCap += buffer[i].linkCap;
            bytes += buffer[i].bytes;
        }

        meanLatency /= mh.dataLength * Messenger::PACKETS_PER_DATA_POINT; // In cycles per packet
        double meanLinkCap = (double) mh.dataLength * Messenger::PACKETS_PER_DATA_POINT / (double) linkCap;  // In packets per available space
        Logl("Bytes: " << bytes << " delta: " << delta);
        long long throughput = 1000000 * bytes / delta; // In bytes per second

        Logl(title << " [" << mh.chainIndex << ", " << mh.appIndex << "] - " << mh.dataLength << " pkts" << setprecision(3)
                   << ", mean = " << getMicro(meanLatency) << "us "
                   << "link = " << meanLinkCap * 100 << "% "
                   << "speed = " << throughput / 1024 << "kB/s");

        double droppedPercent = 100.0f * (double) mh.dropped / (mh.dataLength * Messenger::PACKETS_PER_DATA_POINT);

        if (!seriesName.empty())
        {
            influxdb->write(influxdb::Point{seriesName}
                .addTag("type", title)
                .addTag("chain", to_string(mh.chainIndex))
                .addTag("index", to_string(mh.appIndex))
                .addField("throughput", (throughput / 1024))
                .addField("latency", getMicro(meanLatency))
                .addField("link", meanLinkCap * 100)
                .addField("dropped", droppedPercent)
                .setTimestamp(tp));
        }
    }
}