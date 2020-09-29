#include <interrupts.hpp>
#include <mem/libc.hpp>
#include <errors.hpp>
#include <hwio.hpp>
#include <apic.hpp>
#include <pic.hpp>
#include <cpu.hpp>

extern "C" {
    // Configures the IDTR register with a new IDT.
    void loadidtr(idt::ptr_t *address);

    // Spurious interrupt handler.
    void spuriousisr(void);

    // The common ISR dispatcher, called via commonisr.
    void isrdispatcher(idt::regs64_t *regs) {
        // Check for any available interrupt handlers.
        if(idt::handlers[regs->isr]) idt::handlers[regs->isr](regs);
        else ctxpanic(regs, "unhandled interrupt %ld (0x%lx) raised!", regs->isr, regs->isr);

        // Acknowledge the interrupt if needed.
        if(pic::enabled) pic::sendeoi(regs->isr);
        else apic::writelocal(apic::LAPIC_END_OF_INTERRUPT_REG, 0);
    }
}

namespace idt {
    handler_t handlers[IDTSIZE];
    static entry_t entries[IDTSIZE];
    static ptr_t pointer;

    static void setentry(uint8_t index, void (*entrypoint)(void), uint16_t selector, uint8_t attributes) {
        // Set the entry's offset address (64-bits wide).
        entries[index].offsethi  = ((uint64_t) entrypoint >> 32) & 0xFFFFFFFF;
        entries[index].offsetmid = ((uint64_t) entrypoint >> 16) & 0xFFFF;
        entries[index].offsetlow = (uint64_t) entrypoint & 0xFFFF;

        // Set the entry's attributes and code selector (24-bits wide).
        entries[index].attributes = attributes;
        entries[index].selector = selector;

        // Set the unused areas to zero (40-bits wide).
        entries[index].stacktable = 0;
        entries[index].reservedlow = 0;
        entries[index].reservedhi = 0;
    }

    void registerhandler(uint8_t index, handler_t handler) {
        handlers[index] = handler;
    }

    void initialise(void) {
        // Configure the pointer's address and size.
        pointer.size = sizeof(entry_t) * IDTSIZE - 1;
        pointer.addr = (uint64_t) &entries;

        // Initialise the IDT entries and handler addresses to zero.
        memset(entries, 0, sizeof(entry_t) * IDTSIZE);
        memset(handlers, 0, sizeof(handler_t) * IDTSIZE);
        pic::enable();

        // Map interrupt vectors 0-55 into the IDT.
        setentry(0x00, isr000, 0x08, 0x8E);
        setentry(0x01, isr001, 0x08, 0x8E);
        setentry(0x02, isr002, 0x08, 0x8E);
        setentry(0x03, isr003, 0x08, 0x8E);
        setentry(0x04, isr004, 0x08, 0x8E);
        setentry(0x05, isr005, 0x08, 0x8E);
        setentry(0x06, isr006, 0x08, 0x8E);
        setentry(0x07, isr007, 0x08, 0x8E);
        setentry(0x08, isr008, 0x08, 0x8E);
        setentry(0x09, isr009, 0x08, 0x8E);
        setentry(0x0A, isr010, 0x08, 0x8E);
        setentry(0x0B, isr011, 0x08, 0x8E);
        setentry(0x0C, isr012, 0x08, 0x8E);
        setentry(0x0D, isr013, 0x08, 0x8E);
        setentry(0x0E, isr014, 0x08, 0x8E);
        setentry(0x0F, isr015, 0x08, 0x8E);
        setentry(0x10, isr016, 0x08, 0x8E);
        setentry(0x11, isr017, 0x08, 0x8E);
        setentry(0x12, isr018, 0x08, 0x8E);
        setentry(0x13, isr019, 0x08, 0x8E);
        setentry(0x14, isr020, 0x08, 0x8E);
        setentry(0x15, isr021, 0x08, 0x8E);
        setentry(0x16, isr022, 0x08, 0x8E);
        setentry(0x17, isr023, 0x08, 0x8E);
        setentry(0x18, isr024, 0x08, 0x8E);
        setentry(0x19, isr025, 0x08, 0x8E);
        setentry(0x1A, isr026, 0x08, 0x8E);
        setentry(0x1B, isr027, 0x08, 0x8E);
        setentry(0x1C, isr028, 0x08, 0x8E);
        setentry(0x1D, isr029, 0x08, 0x8E);
        setentry(0x1E, isr030, 0x08, 0x8E);
        setentry(0x1F, isr031, 0x08, 0x8E);
        setentry(0x20, irq000, 0x08, 0x8E);
        setentry(0x21, irq001, 0x08, 0x8E);
        setentry(0x22, irq002, 0x08, 0x8E);
        setentry(0x23, irq003, 0x08, 0x8E);
        setentry(0x24, irq004, 0x08, 0x8E);
        setentry(0x25, irq005, 0x08, 0x8E);
        setentry(0x26, irq006, 0x08, 0x8E);
        setentry(0x27, irq007, 0x08, 0x8E);
        setentry(0x28, irq008, 0x08, 0x8E);
        setentry(0x29, irq009, 0x08, 0x8E);
        setentry(0x2A, irq010, 0x08, 0x8E);
        setentry(0x2B, irq011, 0x08, 0x8E);
        setentry(0x2C, irq012, 0x08, 0x8E);
        setentry(0x2D, irq013, 0x08, 0x8E);
        setentry(0x2E, irq014, 0x08, 0x8E);
        setentry(0x2F, irq015, 0x08, 0x8E);
        setentry(0x30, irq016, 0x08, 0x8E);
        setentry(0x31, irq017, 0x08, 0x8E);
        setentry(0x32, irq018, 0x08, 0x8E);
        setentry(0x33, irq019, 0x08, 0x8E);
        setentry(0x34, irq020, 0x08, 0x8E);
        setentry(0x35, irq021, 0x08, 0x8E);
        setentry(0x36, irq022, 0x08, 0x8E);
        setentry(0x37, irq023, 0x08, 0x8E);

        // Spurious interrupt vectors.
        setentry(0xE7, spuriousisr, 0x08, 0x8E);
        setentry(0xEF, spuriousisr, 0x08, 0x8E);
        setentry(0xFF, spuriousisr, 0x08, 0x8E);

        // Register the generic CPU exception handler for vectors 0-31.
        for(uint8_t i = 0; i < 0x20; i++) registerhandler(i, &cpu::handler);

        // Load the IDTR register and enable interrupts.
        loadidtr(&pointer);
    }
}
