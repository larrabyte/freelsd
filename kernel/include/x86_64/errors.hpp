#ifndef FREELSD_KERNEL_ERRORS_HEADER
#define FREELSD_KERNEL_ERRORS_HEADER

#include <interrupts.hpp>

// Call a kernel panic with processor context.
#define ctxpanic(context, ...) internalpanic(__FILE__, __PRETTY_FUNCTION__, __LINE__, context, __VA_ARGS__)

// Call a kernel panic wthout any processor context.
#define panic(...) internalpanic(__FILE__, __PRETTY_FUNCTION__, __LINE__, nullptr, __VA_ARGS__)

// Halt kernel execution and display a panic screen.
extern "C" __attribute__((noreturn)) void internalpanic(const char *filename, const char *function, int line, idt::regs64_t *regs, const char *format, ...);

#endif
