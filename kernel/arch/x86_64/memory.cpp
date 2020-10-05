#include <mem/libc.hpp>

int memcmp(const void *x, const void *y, size_t n) {
    const char *a = (const char*) x;
    const char *b = (const char*) y;

    while(n-- > 0) {
        // Return non-zero if the values aren't equal.
        if(*a++ != *b++) return (*a > *b) ? 1 : -1;
    }

    return 0;
}

void memset(const void *memory, uint8_t value, size_t n) {
    // Stuff 8-bit value into a 64-bit integer using bitshifting.
    uint64_t bits = (uint64_t) value << 56 | (uint64_t) value << 48 | (uint64_t) value << 40 | (uint64_t) value << 32 | (uint64_t) value << 24 | (uint64_t) value << 16 | (uint64_t) value << 8 | (uint64_t) value;
    char *memoryptr = (char*) memory;

    // Set memory in 8-byte chunks.
    while(n >= sizeof(uint64_t)) {
        *((uint64_t*) memoryptr) = bits;
        memoryptr += sizeof(uint64_t);
        n -= sizeof(uint64_t);
    }

    // Set memory, one byte at a time.
    for(size_t i = 0; i < n; i++) {
        memoryptr[i] = value;
    }
}

void *memcpy(const void *dest, const void *source, size_t n) {
    if(dest == nullptr || source == nullptr || dest == source) return nullptr;
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
