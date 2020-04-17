#include <vector>
#include <fstream>
#include <sstream>

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