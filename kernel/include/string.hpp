#ifndef FREELSD_KERNEL_CSTR_HEADER
#define FREELSD_KERNEL_CSTR_HEADER

#include <stddef.h>
#include <stdint.h>

typedef void (*printf_output_t)(const char);

// Minimal printf() implementation. Supports integers, pointers and strings.
void printf(printf_output_t func, char *format, ...);

// Copies the ASCII representation of num into buffer. Integers passed in with bases other than 10 will be treated as unsigned.
char *itoa(intmax_t num, int base);

// Return the size of a C-style string minus the terminating NULL character.
size_t strlen(const char *str);

#endif