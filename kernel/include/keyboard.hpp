#ifndef FREELSD_KERNEL_KEYBOARD_HEADER
#define FREELSD_KERNEL_KEYBOARD_HEADER

namespace kboard {
    // Initialise the keyboard and register IRQ1 for interrupts.
    void initialise(void);
}

#endif