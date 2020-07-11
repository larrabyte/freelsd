#ifndef FREELSD_KERNEL_CPU_HEADER
#define FREELSD_KERNEL_CPU_HEADER

#include <interrupts.hpp>

namespace cpu {
    // Generic CPU exception handler.
    void handler(idt::regs64_t *regs);
}

#endif
