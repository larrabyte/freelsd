#ifndef FREELSD_KERNEL_TIMER_HEADER
#define FREELSD_KERNEL_TIMER_HEADER

#include <interrupts.hpp>
#include <stdint.h>

// Units of time.
typedef enum timeunits {
    TIMER_MILLISECONDS,
    TIMER_SECONDS,
    TIMER_MINUTES
} timeunits_t;

namespace timer {
    // System tick count since boot.
    extern uint64_t systicks;

    // Timer interrupt handler.
    void handler(idt::regs64_t *regs);

    // Returns the time passed (in units specified) since boot.
    uint64_t sinceboot(timeunits_t unit);

    // Sleep for a specified number of milliseconds.
    void sleep(uint64_t ms);

    // Initialise the Programmable Interval Timer. Values under 3 don't seem to work at this stage.
    void initpit(uint16_t freq);
}

#endif
