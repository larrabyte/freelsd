#ifndef FREELSD_KERNEL_MEMORY_HEADER
#define FREELSD_KERNEL_MEMORY_HEADER

#include <stddef.h>

namespace memory {
    // Sets a block of memory to a certain value.
    void set(const void *memory, size_t n, unsigned char value);

    // Copies n bytes from source -> dest. Unaware of overlapping memory regions.
    void *copy(const void *dest, const void *source, size_t n);
}

#endif