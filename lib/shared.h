#ifndef __SHARED__
#define __SHARED__

#include <iostream>

#include <rte_eal.h>

#include <Port.h>
#include <Ring.h>
#include <Sender.h>
#include <GlobalInfo.h>

#ifdef LOG
    #define Log(x)  (std::cout << x)
#else
    #define Log(x) do{}while(0)
#endif // LOG
#define Logl(x) Log(x << std::endl)


void initEAL(int &argc, char **argv[])
{
    int ret = rte_eal_init(argc, *argv);
    if (ret < 0)
        rte_exit(EXIT_FAILURE, "ERROR: Can't init EAL\n");

    argc -= ret;
    *argv += ret;
}
#endif