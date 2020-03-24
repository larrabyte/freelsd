#ifndef FREELSD_KERNEL_CSTR_HEADER
#define FREELSD_KERNEL_CSTR_HEADER

#include <stddef.h>
#include <stdint.h>

// Copies the ASCII representation of num into buffer. Integers passed in with bases other than 10 will be treated as unsigned.
char *itoa(int num, int base);

// Return the size of a C-style string minus the terminating NULL character.
size_t strlen(const char *str);

#endif