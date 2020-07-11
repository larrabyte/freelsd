#ifndef FREELSD_KERNEL_CSTR_HEADER
#define FREELSD_KERNEL_CSTR_HEADER

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

// Typedef for printk output functions.
typedef void (*printk_output_t)(const char);

// Minimal printf() implementation. Supports integers, pointers and strings.
void printk(printk_output_t func, const char *format, va_list ap);

// Copies the ASCII representation of num into buffer. All integers passed in are treated as unsigned.
char *itoa(uintmax_t num, char *buffer, int base, bool ptrpad);

// Compare strings x and y.
int strcmp(const char *x, const char *y);

// Copy the source string into dest. Only stops at a terminating NULL character!
char *strcpy(char *dest, char *source);

// Return the size of a C-style string minus the terminating NULL character.
size_t strlen(const char *str);

#endif
