#include "GlobalInfo.h"

const std::string GlobalInfo::NAME = "GLOBAL_INFO";

GlobalInfo* GlobalInfo::init(int appCount) 
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

    gi->rxPort = 0;
    gi->txPort = 1;
    gi->appCount = appCount;

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

