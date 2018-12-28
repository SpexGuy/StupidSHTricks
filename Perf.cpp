//
// Created by Martin Wickham on 10/29/2016.
//

#ifdef PERF
#include <iostream>
#include <vector>
#include <cstdio>

#include "Perf.h"

using namespace std;

const int MICROS = 1000000;

LARGE_INTEGER frequency;

int frame_count = 0;

struct PerformanceData {
    const char *name;
    LONGLONG maxTime = 0;
    LONGLONG totalTime = 0;
    LONGLONG maxTimeOneFrame = 0;
    LONGLONG totalTimeThisFrame = 0;
    unsigned int countTotal = 0;
};

vector<PerformanceData> perf_stats;

void initPerformanceData() {
    QueryPerformanceFrequency(&frequency);
    cout << "Recording performance at " << frequency.QuadPart << " ticks per second" << endl;
}

void printPerformanceData() {
    printf("Performance - last %d frames\n", frame_count);
    printf("AVG_STAT  MAX_STAT  PER_FRAME  AVG_FRAME  MAX_FRAME  TAG\n");
    for (const PerformanceData &data : perf_stats) {
        printf("%6llduS  %6llduS  %9.4f  %7llduS  %7llduS  %s\n",
               data.totalTime * MICROS / data.countTotal / frequency.QuadPart,
               data.maxTime * MICROS / frequency.QuadPart,
               float(data.countTotal) / frame_count,
               data.totalTime * MICROS / frame_count / frequency.QuadPart,
               data.maxTimeOneFrame * MICROS / frequency.QuadPart,
               data.name);
    }

    frame_count = 0;
    perf_stats.clear();
}

static void recordStat(PerformanceData &data, const LONGLONG timeElapsed) {
    data.countTotal++;
    data.maxTime = max(data.maxTime, timeElapsed);
    data.totalTimeThisFrame += timeElapsed;
}

void recordPerformanceData(const char *name, const LONGLONG timeElapsed) {
    for (PerformanceData &data : perf_stats) {
        if (data.name == name) { // using == because it's faster and you shouldn't be using the same key multiple times.
            recordStat(data, timeElapsed);
            return;
        }
    }
    perf_stats.emplace_back();
    perf_stats.back().name = name;
    recordStat(perf_stats.back(), timeElapsed);
}

void markPerformanceFrame() {
    for (PerformanceData &data : perf_stats) {
        data.maxTimeOneFrame = max(data.maxTimeOneFrame, data.totalTimeThisFrame);
        data.totalTime += data.totalTimeThisFrame;
        data.totalTimeThisFrame = 0;
    }
    frame_count++;
}

#endif // PERF
