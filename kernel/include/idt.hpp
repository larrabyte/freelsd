#ifndef FREELSD_KERNEL_IDT_HEADER
#define FREELSD_KERNEL_IDT_HEADER

#include <stdint.h>

#define IDTSIZE  256
#define IRQ0     32
#define IRQ1     33
#define IRQ2     34
#define IRQ3     35
#define IRQ4     36
#define IRQ5     37
#define IRQ6     38
#define IRQ7     39
#define IRQ8     40
#define IRQ9     41
#define IRQ10    42
#define IRQ11    43
#define IRQ12    44
#define IRQ13    45
#define IRQ14    46
#define IRQ15    47

namespace idt {
    typedef struct registers {
        uint32_t ds;
        uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
        uint32_t intnum, errcode;
        uint32_t eip, cs, eflags, useresp, ss;
    } registers_t;

    typedef void (*handler_t)(registers_t);

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

    extern handler_t inthandlers[IDTSIZE];
    extern entry_t entries[IDTSIZE];
    extern ptr_t pointer;

    // Register an interrupt handler for a specified interrupt.
    void registerhandler(uint8_t interrupt, handler_t function);

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

        void irq0();
        void irq1();
        void irq2();
        void irq3();
        void irq4();
        void irq5();
        void irq6();
        void irq7();
        void irq8();
        void irq9();
        void irq10();
        void irq11();
        void irq12();
        void irq13();
        void irq14();
        void irq15();
    }
}

#endif