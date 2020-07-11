#ifndef FREELSD_KERNEL_ERRORS_HEADER
#define FREELSD_KERNEL_ERRORS_HEADER

#define panic(...) internalpanic(__FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

extern "C" {
    // Halt kernel execution and display a panic screen.
    __attribute__((noreturn)) void internalpanic(const char *filename, const char *function, int line, const char *format, ...);
}

#endif
