#ifndef FREELSD_KERNEL_TIMER_HEADER
#define FREELSD_KERNEL_TIMER_HEADER

#include <stdint.h>

namespace timer {
    // Initialise the Programmable Interval Timer.
    void initpit(uint32_t frequency);
}

#endif