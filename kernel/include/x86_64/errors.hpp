#ifndef FREELSD_KERNEL_ERRORS_HEADER
#define FREELSD_KERNEL_ERRORS_HEADER

extern "C" {
    // Halt kernel execution and display a panic screen.
    __attribute__((noreturn)) void panic(const char *format, ...);
}

#endif
