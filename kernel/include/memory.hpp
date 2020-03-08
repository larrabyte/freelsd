#ifndef FREELSD_KERNEL_MEMORY_HEADER
#define FREELSD_KERNEL_MEMORY_HEADER

#include <stddef.h>
#include <stdint.h>

namespace memory {
    // Sets a block of memory to a certain value.
    void set(const void *memory, uint8_t value, size_t n);

    // Copies n bytes from source -> dest. Unaware of overlapping memory regions.
    void *copy(const void *dest, const void *source, size_t n);
}

#endif