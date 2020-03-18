#ifndef FREELSD_KERNEL_KEYBOARD_HEADER
#define FREELSD_KERNEL_KEYBOARD_HEADER

#include <idt.hpp>

namespace kboard {
    // Keyboard interrupt handler.
    void handler(idt::registers_t *regs);

    // Initialise the keyboard and register IRQ1 for interrupts.
    void initialise(void);
}

#endif