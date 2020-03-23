#include <gdt.hpp>

namespace gdt {
    entry_t entries[GDTSIZE];
    ptr_t pointer;

    static void setgate(int32_t index, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity) {
        entries[index].granularity   = (limit >> 16) & 0x0F;
        entries[index].granularity  |= granularity & 0xF0;
        entries[index].limitlow      = (limit & 0xFFFF);
        entries[index].access        = access;

        entries[index].basehigh      = (base >> 24) & 0xFF;
        entries[index].basemiddle    = (base >> 16) & 0xFF;
        entries[index].baselow       = (base & 0xFFFF);
    }

    void initialise(void) {
        pointer.limit = sizeof(gdt::entry_t) * GDTSIZE - 1;
        pointer.base = (uint32_t) &entries;

        setgate(0, 0, 0, 0, 0);                 // Null segment entry.
        setgate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);  // Kernel code segment entry.
        setgate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);  // Kernel data segment entry.
        setgate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);  // User code segment entry.
        setgate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);  // User data segment entry.

        gdtflush((uintptr_t) &pointer);
    }
}

