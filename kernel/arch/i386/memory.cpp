#include <memory.hpp>
#include <stdint.h>

void memory::set(const void *memory, size_t n, uint8_t value) {
    char *memcharptr = (char*) memory;
    for(size_t i = 0; i < n; i++) memcharptr[i] = value;
}

void *memory::copy(const void *dest, const void *source, size_t n) {
    if(dest == NULL || source == NULL || dest == source) return NULL;
    char *src = (char*) source;
    char *dst = (char*) dest;

    while(n >= sizeof(uint64_t)) {
        *((uint64_t*) dst) = *((uint64_t*) src);
        dst += sizeof(uint64_t);
        src += sizeof(uint64_t);
        n -= sizeof(uint64_t);
    }

    for(size_t i = 0; i < n; i++) {
        dst[i] = src[i];
    }

    return (void*) dest;
}