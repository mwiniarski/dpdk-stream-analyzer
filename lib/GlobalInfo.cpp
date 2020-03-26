#include "GlobalInfo.h"

const std::string GlobalInfo::NAME = "GLOBAL_INFO";

GlobalInfo* GlobalInfo::init(std::vector<int> chainSizes)
{
    // Keep port info in system-wise accessible way
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

    gi->chainCount = chainSizes.size();
    for (uint i = 0; i < chainSizes.size(); i++)
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

bool GlobalInfo::isLastInChain(uint appIndex, uint chainIndex)
{
    return appIndex + 1 == this->appsInChain[chainIndex];
}
