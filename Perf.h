//
// Created by Martin Wickham on 10/25/2016.
//

#ifndef STUPIDSHTRICKS_PERF_H
#define STUPIDSHTRICKS_PERF_H


#include <afxres.h>

void initPerformanceData();
void printPerformanceData();
void recordPerformanceData(const char *name, const LONGLONG timeElapsed);
void markPerformanceFrame();

class Perf {
private:
    const char * const name;
    LARGE_INTEGER startTime;

public:
    Perf(const char *name) :
            name(name)
    {
        QueryPerformanceCounter(&startTime);
    }

    ~Perf() {
#ifdef PERF
        LARGE_INTEGER endTime;
        QueryPerformanceCounter(&endTime);
        recordPerformanceData(name, endTime.QuadPart - startTime.QuadPart);
#endif
    }
};


#endif //STUPIDSHTRICKS_PERF_H
