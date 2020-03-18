#include <gdt.hpp>

extern "C" void gdtflush(void);

gdt::entry_t gdtarray[GDTSIZE];
gdt::ptr_t gdtptr;

static void setgate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity) {
    gdtarray[num].granularity   = (limit >> 16) & 0x0F;
    gdtarray[num].granularity  |= granularity & 0xF0;
    gdtarray[num].limitlow      = (limit & 0xFFFF);
    gdtarray[num].access        = access;

    gdtarray[num].basehigh      = (base >> 24) & 0xFF;
    gdtarray[num].basemiddle    = (base >> 16) & 0xFF;
    gdtarray[num].baselow       = (base & 0xFFFF);
}

void gdt::initialise(void) {
    gdtptr.limit = sizeof(gdt::entry_t) * GDTSIZE - 1;
    gdtptr.base = (uint32_t) &gdtarray;

    setgate(0, 0, 0, 0, 0);                 // Null segment entry.
    setgate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);  // Kernel code segment entry.
    setgate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);  // Kernel data segment entry.
    setgate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);  // User code segment entry.
    setgate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);  // User data segment entry.

    gdtflush();
}