#ifndef FREELSD_KERNEL_IDT_HEADER
#define FREELSD_KERNEL_IDT_HEADER

#include <stdint.h>

#define IDTSIZE 256

namespace idt {
    typedef struct entry {
        uint16_t baselow;
        uint16_t selector;
        uint8_t zero;
        uint8_t flags;
        uint16_t basehigh;
    } __attribute__((packed)) entry_t;

    typedef struct ptr {
        uint16_t limit;
        uint32_t base;
    } __attribute__((packed)) ptr_t;

    extern entry_t array[IDTSIZE];
    extern ptr_t pointer;

    // Initialise the interrupt descriptor table.
    void initialise(void);

    extern "C" {
        void isr0();
        void isr1();
        void isr2();
        void isr3();
        void isr4();
        void isr5();
        void isr6();
        void isr7();
        void isr8();
        void isr9();
        void isr10();
        void isr11();
        void isr12();
        void isr13();
        void isr14();
        void isr15();
        void isr16();
        void isr17();
        void isr18();
        void isr19();
        void isr20();
        void isr21();
        void isr22();
        void isr23();
        void isr24();
        void isr25();
        void isr26();
        void isr27();
        void isr28();
        void isr29();
        void isr30();
        void isr31();
    }
}

#endif