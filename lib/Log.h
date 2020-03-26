#ifndef __LOG__
#define __LOG__

#include <iostream>

#ifdef LOG
    #define Log(x)  (std::cout << x)
#else
    #define Log(x) do{}while(0)
#endif // LOG
#define Logl(x) Log(x << std::endl)

#endif