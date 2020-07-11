#ifndef FREELSD_KERNEL_INTERRUPTS_HEADER
#define FREELSD_KERNEL_INTERRUPTS_HEADER

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

extern "C" {
    void isr00(void);  // Interrupt Service Routine 00: Divide-by-zero.
    void isr01(void);  // Interrupt Service Routine 01: Debug exception.
    void isr02(void);  // Interrupt Service Routine 02: Non-maskable interrupt.
    void isr03(void);  // Interrupt Service Routine 03: Breakpoint exception.
    void isr04(void);  // Interrupt Service Routine 04: Overflow exception.
    void isr05(void);  // Interrupt Service Routine 05: Bound range exceeded.
    void isr06(void);  // Interrupt Service Routine 06: Invalid opcode exception.
    void isr07(void);  // Interrupt Service Routine 07: Device not available.
    void isr08(void);  // Interrupt Service Routine 08: Double fault exception.
    void isr09(void);  // Interrupt Service Routine 09: Reserved.
    void isr10(void);  // Interrupt Service Routine 10: Invalid task state segment.
    void isr11(void);  // Interrupt Service Routine 11: Segment not present.
    void isr12(void);  // Interrupt Service Routine 12: Stack-segment fault.
    void isr13(void);  // Interrupt Service Routine 13: General protection fault.
    void isr14(void);  // Interrupt Service Routine 14: Page fault.
    void isr15(void);  // Interrupt Service Routine 15: Reserved.
    void isr16(void);  // Interrupt Service Routine 16: x87 Floating-Point exception.
    void isr17(void);  // Interrupt Service Routine 17: Alignment check exception.
    void isr18(void);  // Interrupt Service Routine 18: Machine check exception.
    void isr19(void);  // Interrupt Service Routine 19: SIMD Floating-Point exception.
    void isr20(void);  // Interrupt Service Routine 20: Virtualisation exception.
    void isr21(void);  // Interrupt Service Routine 21: Reserved.
    void isr22(void);  // Interrupt Service Routine 22: Reserved.
    void isr23(void);  // Interrupt Service Routine 23: Reserved.
    void isr24(void);  // Interrupt Service Routine 24: Reserved.
    void isr25(void);  // Interrupt Service Routine 25: Reserved.
    void isr26(void);  // Interrupt Service Routine 26: Reserved.
    void isr27(void);  // Interrupt Service Routine 27: Reserved.
    void isr28(void);  // Interrupt Service Routine 28: Reserved.
    void isr29(void);  // Interrupt Service Routine 29: Reserved.
    void isr30(void);  // Interrupt Service Routine 30: Security exception.
    void isr31(void);  // Interrupt Service Routine 31: Reserved.

    void irq00(void);
    void irq01(void);
    void irq02(void);
    void irq03(void);
    void irq04(void);
    void irq05(void);
    void irq06(void);
    void irq07(void);
    void irq08(void);
    void irq09(void);
    void irq10(void);
    void irq11(void);
    void irq12(void);
    void irq13(void);
    void irq14(void);
    void irq15(void);
}

namespace idt {
    typedef struct registers64 {
        uint64_t r15, r14, r13, r12, r11, r10, r9, r8;  // Pushed manually by the pushaq macro.
        uint64_t rdi, rsi, rbp, rbx, rdx, rcx, rax;     // Pushed manually by the pushaq macro.
        uint64_t isr, err;                              // Pushed manually by ISR/IRQ macro.
        uint64_t rip, cs, rflags, rsp, ss;              // Pushed automatically by the CPU.
    } regs64_t;

    typedef struct entry {
        uint16_t offsetlow;      // ISR entry point, bits 00-15.
        uint16_t selector;       // GDT code segment selector.
        uint8_t stacktable: 3;   // The interrupt stack table offset.
        uint8_t reservedlow: 5;  // Reserved, bits are zeroed.
        uint8_t attributes;      // Types and attributes.
        uint16_t offsetmid;      // ISR entry point, bits 16-31.
        uint32_t offsethi;       // ISR entry point, bits 32-63.
        uint32_t reservedhi;     // Reserved, bits are zeroed.
    } entry_t;

    typedef struct ptr {
        uint16_t size;  // The size of the IDT, in bytes.
        uint64_t addr;  // The linear address of the IDT.
    } __attribute__((packed)) ptr_t;

    // Function definition for an interrupt handler.
    typedef void (*handler_t)(regs64_t*);

    // Array of programmable interrupt handlers.
    extern handler_t handlers[IDTSIZE];

    // Register an interrupt handler at the specified index.
    void registerhandler(uint8_t index, handler_t handler);

    // Initialise the interrupt descriptor table.
    void initialise(void);
}

#endif
