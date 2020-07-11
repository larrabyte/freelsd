#ifndef FREELSD_KERNEL_ERRORS_HEADER
#define FREELSD_KERNEL_ERRORS_HEADER

#include <interrupts.hpp>

// Call a kernel panic with processor context.
#define ctxpanic(context, ...) internalpanic(__FILE__, __FUNCTION__, __LINE__, context, __VA_ARGS__)

// Call a kernel panic wthout any processor context.
#define panic(...) internalpanic(__FILE__, __FUNCTION__, __LINE__, nullptr, __VA_ARGS__)


extern "C" {
    // Halt kernel execution and display a panic screen.
    __attribute__((noreturn)) void internalpanic(const char *filename, const char *function, int line, idt::regs64_t *regs, const char *format, ...);
}

#endif
