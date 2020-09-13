#ifndef FREELSD_KERNEL_MEMORY_LIBC_HEADER
#define FREELSD_KERNEL_MEMORY_LIBC_HEADER

#include <stddef.h>
#include <stdint.h>

// Compare two blocks of memory, x and y for n bytes.
int memcmp(const void *x, const void *y, size_t n);

// Sets a block of memory to a certain value.
void memset(const void *memory, uint8_t value, size_t n);

// Copies n bytes from source -> dest. Unaware of overlapping memory regions.
void *memcpy(const void *dest, const void *source, size_t n);

#endif
