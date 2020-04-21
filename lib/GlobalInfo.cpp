#include "GlobalInfo.h"

const std::string GlobalInfo::NAME = "GLOBAL_INFO";
const std::string GlobalInfo::STATS_NAME = "stats3";
const std::string GlobalInfo::MEMPOOL = "MBUF_POOL";

GlobalInfo* GlobalInfo::init(std::vector<int> chainSizes)
{
    // Allocate system-wise accessible memory
    const rte_memzone *memzone = rte_memzone_reserve(
                                        NAME.c_str(),
                                        sizeof(GlobalInfo),
                                        rte_socket_id(),
                                        0);

    if (!memzone)
        rte_exit(EXIT_FAILURE, "ERROR: Can't reserve memory zone for port info\n");

    GlobalInfo* gi = (GlobalInfo*) memzone->addr;
    std::memset(gi, 0, sizeof(GlobalInfo));

    // Set app data
    if (chainSizes.size() > MAX_CHAINS)
        rte_exit(EXIT_FAILURE, "ERROR: Too many chains specified!\n");

    // Copy values from vector into preallocated shared memory
    gi->chainCount = chainSizes.size();
    for (unsigned i = 0; i < chainSizes.size(); i++)
    {
        if (chainSizes[i] < 1 || chainSizes[i] > MAX_APPS)
            rte_exit(EXIT_FAILURE, "ERROR: Chain size must be > 0 and <= 8 !\n");

        gi->appsInChain[i] = chainSizes[i];
    }

    // Set port data
    gi->rxPort = 0;
    gi->txPort = 1;

    return gi;
}

GlobalInfo* GlobalInfo::get()
{
    // Retrieve port info shared by server
    const rte_memzone *memzone = rte_memzone_lookup(NAME.c_str());

    if (!memzone)
        rte_exit(EXIT_FAILURE, "ERROR: Can't read '%s' memzone\n", NAME.c_str());

    return (GlobalInfo *) memzone->addr;
}

bool GlobalInfo::isLastInChain(int appIndex, int chainIndex)
{
    return appIndex + 1 == this->appsInChain[chainIndex];
}
