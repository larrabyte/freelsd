#ifndef FREELSD_KERNEL_TIMER_HEADER
#define FREELSD_KERNEL_TIMER_HEADER

#include <interrupts.hpp>
#include <stdint.h>

namespace timer {
    // System tick count since boot.
    extern uint64_t systicks;

    // Timer interrupt handler.
    void handler(idt::regs32_t *regs);

    // Sleep for a specified number of milliseconds.
    void sleep(uint64_t milliseconds);

    // Initialise the Programmable Interval Timer, given a certain frequency. Values under 3 don't seem to work at this stage.
    void initpit(uint32_t frequency);
}

#endif