#pragma once

#include <stdint.h>

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

    extern gdt::entry_t array[5];
    extern gdt::ptr_t pointer;

    // Initialise the global descriptor table.
    void initialise(void);
}