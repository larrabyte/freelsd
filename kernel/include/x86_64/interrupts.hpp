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
#define IRQ16    48
#define IRQ17    49
#define IRQ18    50
#define IRQ19    51
#define IRQ20    52
#define IRQ21    53
#define IRQ22    54
#define IRQ23    55

extern "C" {
    void isr000(void); // Interrupt Service Routine 000: Divide-by-zero.
    void isr001(void); // Interrupt Service Routine 001: Debug exception.
    void isr002(void); // Interrupt Service Routine 002: Non-maskable interrupt.
    void isr003(void); // Interrupt Service Routine 003: Breakpoint exception.
    void isr004(void); // Interrupt Service Routine 004: Overflow exception.
    void isr005(void); // Interrupt Service Routine 005: Bound range exceeded.
    void isr006(void); // Interrupt Service Routine 006: Invalid opcode exception.
    void isr007(void); // Interrupt Service Routine 007: Device not available.
    void isr008(void); // Interrupt Service Routine 008: Double fault exception.
    void isr009(void); // Interrupt Service Routine 009: Reserved.
    void isr010(void); // Interrupt Service Routine 010: Invalid task state segment.
    void isr011(void); // Interrupt Service Routine 011: Segment not present.
    void isr012(void); // Interrupt Service Routine 012: Stack-segment fault.
    void isr013(void); // Interrupt Service Routine 013: General protection fault.
    void isr014(void); // Interrupt Service Routine 014: Page fault.
    void isr015(void); // Interrupt Service Routine 015: Reserved.
    void isr016(void); // Interrupt Service Routine 016: x87 Floating-Point exception.
    void isr017(void); // Interrupt Service Routine 017: Alignment check exception.
    void isr018(void); // Interrupt Service Routine 018: Machine check exception.
    void isr019(void); // Interrupt Service Routine 019: SIMD Floating-Point exception.
    void isr020(void); // Interrupt Service Routine 020: Virtualisation exception.
    void isr021(void); // Interrupt Service Routine 021: Reserved.
    void isr022(void); // Interrupt Service Routine 022: Reserved.
    void isr023(void); // Interrupt Service Routine 023: Reserved.
    void isr024(void); // Interrupt Service Routine 024: Reserved.
    void isr025(void); // Interrupt Service Routine 025: Reserved.
    void isr026(void); // Interrupt Service Routine 026: Reserved.
    void isr027(void); // Interrupt Service Routine 027: Reserved.
    void isr028(void); // Interrupt Service Routine 028: Reserved.
    void isr029(void); // Interrupt Service Routine 029: Reserved.
    void isr030(void); // Interrupt Service Routine 030: Security exception.
    void isr031(void); // Interrupt Service Routine 031: Reserved.

    void irq000(void);
    void irq001(void);
    void irq002(void);
    void irq003(void);
    void irq004(void);
    void irq005(void);
    void irq006(void);
    void irq007(void);
    void irq008(void);
    void irq009(void);
    void irq010(void);
    void irq011(void);
    void irq012(void);
    void irq013(void);
    void irq014(void);
    void irq015(void);
    void irq016(void);
    void irq017(void);
    void irq018(void);
    void irq019(void);
    void irq020(void);
    void irq021(void);
    void irq022(void);
    void irq023(void);
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
