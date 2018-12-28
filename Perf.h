//
// Created by Martin Wickham on 10/25/2016.
//

#ifndef STUPIDSHTRICKS_PERF_H
#define STUPIDSHTRICKS_PERF_H

#ifdef PERF

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
        LARGE_INTEGER endTime;
        QueryPerformanceCounter(&endTime);
        recordPerformanceData(name, endTime.QuadPart - startTime.QuadPart);
    }
};

#else

static inline void initPerformanceData() {}
static inline void printPerformanceData() {}
static inline void recordPerformanceData(const char *name, const long long timeElapsed) {}
static inline void markPerformanceFrame() {}

struct Perf {
  Perf(const char *name) {}
};

#endif //PERF

#endif //STUPIDSHTRICKS_PERF_H
