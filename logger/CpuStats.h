#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>

const int CPU_COUNT = 5;

struct CpuStats
{
    int total = 0;
    int user;
    int system;
    int idle;
};

struct DiffStats
{
    double user;
    double system;
    double idle;
};

const int NUM_CPU_STATES = 10;

std::vector<CpuStats> readStatsCPU()
{
	std::ifstream fileStat("/proc/stat");
    std::vector<CpuStats> cpus;

	std::string line;
    bool first = true;

	while(std::getline(fileStat, line))
	{
		// cpu stats line found
		if(!line.compare(0, 3, "cpu"))
		{
            if (first)
            {
                first = false;
                continue;
            }

			std::istringstream ss(line);
            
            int num; std::string s;
            cpus.emplace_back();
            auto & stats = cpus.back();
			
            // skip first word
            ss >> s;

			// read times
			for(int i = 0; i < NUM_CPU_STATES; ++i)
			{
                ss >> num;

                if (i == 0)
                    stats.user = num;

                if (i == 2)
                    stats.system = num;

                if (i == 3)
                    stats.idle = num;

                stats.total += num;
            }
		}
        else if (!first)
        {
            break;
        }
	}

    return cpus;
}

std::vector<DiffStats> calculateUsage(std::vector<CpuStats>& olds, std::vector<CpuStats>& news)
{
    std::vector<DiffStats> diffs;

    for (uint i = 0; i < olds.size(); i++)
    {
        int totalDiff = news[i].total - olds[i].total;
        double user = 100.0f * (news[i].user - olds[i].user) / totalDiff;
        double system = 100.0f * (news[i].system - olds[i].system) / totalDiff;
        double idle = 100.0f * (news[i].idle - olds[i].idle) / totalDiff;

        diffs.push_back({user, system, idle});
    }

    return diffs;
}

void runCPUMeasurements(std::string& url)
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
        auto now = std::chrono::system_clock::now();
        oldStats = newStats;

        for (uint i = 0; i < diffs.size(); i++)
        {
            influxdb->write(influxdb::Point{"CPUstats"}
                .addTag("cpu", std::to_string(i))
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