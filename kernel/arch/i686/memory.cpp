#include <memory.hpp>
#include <stdint.h>

void memory::set(const void *memory, uint8_t value, size_t n) {
    uint32_t manybits  = (value << 24) | (value << 16) | (value << 8) | value;
    char *memoryptr = (char*) memory;

    while(n >= sizeof(uint32_t)) {
        *((uint32_t*) memoryptr) = manybits;
        memoryptr += sizeof(uint32_t);
        n -= sizeof(uint32_t);
    }

    for(size_t i = 0; i < n; i++) {
        memoryptr[i] = value;
    }
}

void *memory::copy(const void *dest, const void *source, size_t n) {
    if(dest == NULL || source == NULL || dest == source) return NULL;
    char *src = (char*) source;
    char *dst = (char*) dest;

    // Copy in 8-byte chunks.
    while(n >= sizeof(uint64_t)) {
        *((uint64_t*) dst) = *((uint64_t*) src);
        dst += sizeof(uint64_t);
        src += sizeof(uint64_t);
        n -= sizeof(uint64_t);
    }

    // Copy in one-byte chunks.
    for(size_t i = 0; i < n; i++) {
        dst[i] = src[i];
    }

    return (void*) dest;
}