#ifndef FREELSD_KERNEL_GDT_HEADER
#define FREELSD_KERNEL_GDT_HEADER

#include <stdint.h>

#define GDTSIZE 5

extern "C" {
    void gdtflush(uintptr_t pointer);
}

namespace gdt {
    typedef struct entry {
        uint16_t limitlow;
        uint16_t baselow;
        uint8_t basemiddle;
        uint8_t access;
        uint8_t granularity;
        uint8_t basehigh;
    } __attribute__((packed)) entry_t;

    typedef struct ptr {
        uint16_t limit;
        uint32_t base;
    } __attribute__((packed)) ptr_t;

    extern entry_t entries[GDTSIZE];
    extern ptr_t pointer;

    // Initialise the global descriptor table.
    void initialise(void);
}

#endif