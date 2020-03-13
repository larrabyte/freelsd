#ifndef FREELSD_KERNEL_TIMER_HEADER
#define FREELSD_KERNEL_TIMER_HEADER

#include <stdint.h>
#include <idt.hpp>

namespace timer {
    // Timer interrupt handler.
    void handler(idt::registers_t regs);

    // Initialise the Programmable Interval Timer, given a certain frequency. Values under 3 don't seem to work at this stage.
    void initpit(uint32_t frequency);

    // Sleep for a specified number of milliseconds.
    void sleep(uint64_t milliseconds);
}

#endif