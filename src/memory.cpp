#include "head/memory.hpp"
#include <stdint.h>

void memory::set(const void *memory, size_t n, uint8_t value) {
    char *memcharptr = (char*) memory;
    while(n) memcharptr[n--] = value;
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

/* void *memcpy(void* dest, const void* src, size_t count) {
    const char *sp = (char*)src;
    char *dp = (char *)dest;
    for(size_t i = count; i >= sizeof(uint64_t); i = count){
        *((uint64_t*)dp) = *((uint64_t*)sp);
        sp = sp + sizeof(uint64_t);
        dp = dp + sizeof(uint64_t);
        count -= sizeof(uint64_t);
    }
    for(size_t i = count; i >= 4; i = count){
        *((uint32_t*)dp) = *((uint32_t*)sp);
        sp = sp + 4;
        dp = dp + 4;
        count -= 4;
    }
    for (size_t i = count; i > 0; i = count){
        dp[i] = sp[i];
        count--;
    } 
    return dest;
} */