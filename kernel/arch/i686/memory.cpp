#include <mem/libc.hpp>
#include <stdint.h>

void memset(const void *memory, uint8_t value, size_t n) {
    // Stuff 8-bit value into a 64-bit integer using bitshifting.
    uint32_t manybits = (value << 24) | (value << 16) | (value << 8) | value;
    uint64_t morebits = ((uint64_t) manybits << 32) | manybits;
    char *memoryptr = (char*) memory;

    // Set memory in 8-byte chunks.
    while(n >= sizeof(uint64_t)) {
        *((uint64_t*) memoryptr) = morebits;
        memoryptr += sizeof(uint64_t);
        n -= sizeof(uint64_t);
    }

    // Set memory, one byte at a time.
    for(size_t i = 0; i < n; i++) {
        memoryptr[i] = value;
    }
}

void *memcpy(const void *dest, const void *source, size_t n) {
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
