//
// Created by Martin Wickham on 10/25/2016.
//

#ifndef STUPIDSHTRICKS_PERF_H
#define STUPIDSHTRICKS_PERF_H


#include <ctime>
#include <iostream>


class Perf {
private:
    const char * const name;
    const volatile clock_t startTime;

public:
    Perf(const char *name) :
            name(name),
            startTime(clock())
    {}

    ~Perf() {
#ifdef PERF
        clock_t endTime = clock();
        std::cout << name << " : " << endTime - startTime << std::endl;
#endif
    }
};


#endif //STUPIDSHTRICKS_PERF_H
