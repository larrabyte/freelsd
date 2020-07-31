#include <interrupts.hpp>
#include <mem/libc.hpp>
#include <errors.hpp>
#include <hwio.hpp>
#include <cpu.hpp>

extern "C" {
    // Configures the IDTR register with a new IDT.
    void loadidtr(idt::ptr_t *address);

    // The common ISR dispatcher, called via commonisr.
    void isrdispatcher(idt::regs64_t *regs) {
        // Check if it's a spurious IRQ.
        if(regs->isr == 7 || regs->isr == 15) {
            uint16_t picisr = idt::readpicregs(PIC_READ_ISR);
            if((picisr & (1 << regs->isr)) == 0) {
                if(regs->isr == 15) outportb(0x20, 0x20);
                return;
            }
        }

        // Not spurious, check for any available interrupt handlers.
        if(idt::handlers[regs->isr]) idt::handlers[regs->isr](regs);
        else ctxpanic(regs, "unhandled interrupt %ld (0x%lx) raised!", regs->isr, regs->isr);

        // Acknowledge the interrupt, if required send to both slave and master PICs.
        if(regs->isr >= 40) outportb(0xA0, 0x20);
        outportb(0x20, 0x20);
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

    uint16_t readpicregs(uint8_t command) {
        outportb(MASTER_PIC_COMMAND, command);
        outportb(SLAVE_PIC_COMMAND, command);
        return (inportb(MASTER_PIC_COMMAND) << 8) | inportb(SLAVE_PIC_COMMAND);
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

        // Remap the PICs to work with interrupts from 32 onwards.
        outportb(0x20, 0x11);
        outportb(0xA0, 0x11);
        outportb(0x21, 0x20);
        outportb(0xA1, 0x28);
        outportb(0x21, 0x04);
        outportb(0xA1, 0x02);
        outportb(0x21, 0x01);
        outportb(0xA1, 0x01);
        outportb(0x21, 0x00);
        outportb(0xA1, 0x00);

        // Map ISRs 0-47 into the IDT.
        setentry(0x00, isr00, 0x08, 0x8E);
        setentry(0x01, isr01, 0x08, 0x8E);
        setentry(0x02, isr02, 0x08, 0x8E);
        setentry(0x03, isr03, 0x08, 0x8E);
        setentry(0x04, isr04, 0x08, 0x8E);
        setentry(0x05, isr05, 0x08, 0x8E);
        setentry(0x06, isr06, 0x08, 0x8E);
        setentry(0x07, isr07, 0x08, 0x8E);
        setentry(0x08, isr08, 0x08, 0x8E);
        setentry(0x09, isr09, 0x08, 0x8E);
        setentry(0x0A, isr10, 0x08, 0x8E);
        setentry(0x0B, isr11, 0x08, 0x8E);
        setentry(0x0C, isr12, 0x08, 0x8E);
        setentry(0x0D, isr13, 0x08, 0x8E);
        setentry(0x0E, isr14, 0x08, 0x8E);
        setentry(0x0F, isr15, 0x08, 0x8E);
        setentry(0x10, isr16, 0x08, 0x8E);
        setentry(0x11, isr17, 0x08, 0x8E);
        setentry(0x12, isr18, 0x08, 0x8E);
        setentry(0x13, isr19, 0x08, 0x8E);
        setentry(0x14, isr20, 0x08, 0x8E);
        setentry(0x15, isr21, 0x08, 0x8E);
        setentry(0x16, isr22, 0x08, 0x8E);
        setentry(0x17, isr23, 0x08, 0x8E);
        setentry(0x18, isr24, 0x08, 0x8E);
        setentry(0x19, isr25, 0x08, 0x8E);
        setentry(0x1A, isr26, 0x08, 0x8E);
        setentry(0x1B, isr27, 0x08, 0x8E);
        setentry(0x1C, isr28, 0x08, 0x8E);
        setentry(0x1D, isr29, 0x08, 0x8E);
        setentry(0x1E, isr30, 0x08, 0x8E);
        setentry(0x1F, isr31, 0x08, 0x8E);
        setentry(0x20, irq00, 0x08, 0x8E);
        setentry(0x21, irq01, 0x08, 0x8E);
        setentry(0x22, irq02, 0x08, 0x8E);
        setentry(0x23, irq03, 0x08, 0x8E);
        setentry(0x24, irq04, 0x08, 0x8E);
        setentry(0x25, irq05, 0x08, 0x8E);
        setentry(0x26, irq06, 0x08, 0x8E);
        setentry(0x27, irq07, 0x08, 0x8E);
        setentry(0x28, irq08, 0x08, 0x8E);
        setentry(0x29, irq09, 0x08, 0x8E);
        setentry(0x2A, irq10, 0x08, 0x8E);
        setentry(0x2B, irq11, 0x08, 0x8E);
        setentry(0x2C, irq12, 0x08, 0x8E);
        setentry(0x2D, irq13, 0x08, 0x8E);
        setentry(0x2E, irq14, 0x08, 0x8E);
        setentry(0x2F, irq15, 0x08, 0x8E);

        // Register the generic CPU exception handler for interrupts 0-31.
        registerhandler(0x00, &cpu::handler);
        registerhandler(0x01, &cpu::handler);
        registerhandler(0x02, &cpu::handler);
        registerhandler(0x03, &cpu::handler);
        registerhandler(0x04, &cpu::handler);
        registerhandler(0x05, &cpu::handler);
        registerhandler(0x06, &cpu::handler);
        registerhandler(0x07, &cpu::handler);
        registerhandler(0x08, &cpu::handler);
        registerhandler(0x09, &cpu::handler);
        registerhandler(0x0A, &cpu::handler);
        registerhandler(0x0B, &cpu::handler);
        registerhandler(0x0C, &cpu::handler);
        registerhandler(0x0D, &cpu::handler);
        registerhandler(0x0E, &cpu::handler);
        registerhandler(0x0F, &cpu::handler);
        registerhandler(0x10, &cpu::handler);
        registerhandler(0x11, &cpu::handler);
        registerhandler(0x12, &cpu::handler);
        registerhandler(0x13, &cpu::handler);
        registerhandler(0x14, &cpu::handler);
        registerhandler(0x15, &cpu::handler);
        registerhandler(0x16, &cpu::handler);
        registerhandler(0x17, &cpu::handler);
        registerhandler(0x18, &cpu::handler);
        registerhandler(0x19, &cpu::handler);
        registerhandler(0x1A, &cpu::handler);
        registerhandler(0x1B, &cpu::handler);
        registerhandler(0x1C, &cpu::handler);
        registerhandler(0x1D, &cpu::handler);
        registerhandler(0x1E, &cpu::handler);
        registerhandler(0x1F, &cpu::handler);

        // Set the IDTR register and enable interrupts.
        loadidtr(&pointer);
    }
}
