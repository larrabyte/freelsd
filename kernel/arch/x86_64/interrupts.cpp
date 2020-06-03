#include <interrupts.hpp>
#include <mem/libc.hpp>
#include <serial.hpp>
#include <hwio.hpp>

extern "C" void isrhandler(idt::registers_t *regs) {
    // Call the specified interrupt handler or print an error.
    if(idt::handlers[regs->isr]) idt::handlers[regs->isr](regs);
    else serial::printf("[isr] unhandled interrupt: %d\n", regs->isr);

    // Acknowledge the interrupt, if required send to both slave and master PICs.
    if(regs->isr >= 40) outportb(0xA0, 0x20);
    outportb(0x20, 0x20);
}

namespace idt {
    handler_t handlers[IDTSIZE];
    entry_t entries[IDTSIZE];
    ptr_t pointer;

    static void setentry(uint8_t index, uint64_t base, uint16_t selector, uint8_t ist, uint8_t attributes) {
        entries[index].basehigh = base >> 32;
        entries[index].basemid = base >> 16;
        entries[index].baselow = base;
        entries[index].istoffset = ist;
        entries[index].selector = selector;
        entries[index].attributes = attributes;
    }

    void registerhandler(uint8_t interrupt, handler_t function) {
        handlers[interrupt] = function;
    }

    void initialise(void) {
        pointer.limit = sizeof(entry_t) * IDTSIZE - 1;
        pointer.base = (uint64_t) &entries;

        // Initialise arrays to zero.
        memset(handlers, 0, sizeof(handler_t) * IDTSIZE);
        memset(entries, 0, sizeof(entry_t) * IDTSIZE);

        // Remap the PICs to work with interrupts from 32 onwards.
        outportb(0x20, 0x11);
        outportb(0xA0, 0x11);
        outportb(0x21, 0x20);
        outportb(0xA1, 0x28);
        outportb(0x21, 0x04);
        outportb(0xA1, 0x02);
        outportb(0x21, 0x01);
        outportb(0xA1, 0x01);
        outportb(0x21, 0x0);
        outportb(0xA1, 0x0);

        // Map ISRs 0-47 to their respective functions.
        setentry(0, (uint64_t) isr0, 0x08, 0x00, 0x8E);
        setentry(1, (uint64_t) isr1, 0x08, 0x00, 0x8E);
        setentry(2, (uint64_t) isr2, 0x08, 0x00, 0x8E);
        setentry(3, (uint64_t) isr3, 0x08, 0x00, 0x8E);
        setentry(4, (uint64_t) isr4, 0x08, 0x00, 0x8E);
        setentry(5, (uint64_t) isr5, 0x08, 0x00, 0x8E);
        setentry(6, (uint64_t) isr6, 0x08, 0x00, 0x8E);
        setentry(7, (uint64_t) isr7, 0x08, 0x00, 0x8E);
        setentry(8, (uint64_t) isr8, 0x08, 0x00, 0x8E);
        setentry(9, (uint64_t) isr9, 0x08, 0x00, 0x8E);
        setentry(10, (uint64_t) isr10, 0x08, 0x00, 0x8E);
        setentry(11, (uint64_t) isr11, 0x08, 0x00, 0x8E);
        setentry(12, (uint64_t) isr12, 0x08, 0x00, 0x8E);
        setentry(13, (uint64_t) isr13, 0x08, 0x00, 0x8E);
        setentry(14, (uint64_t) isr14, 0x08, 0x00, 0x8E);
        setentry(15, (uint64_t) isr15, 0x08, 0x00, 0x8E);
        setentry(16, (uint64_t) isr16, 0x08, 0x00, 0x8E);
        setentry(17, (uint64_t) isr17, 0x08, 0x00, 0x8E);
        setentry(18, (uint64_t) isr18, 0x08, 0x00, 0x8E);
        setentry(19, (uint64_t) isr19, 0x08, 0x00, 0x8E);
        setentry(20, (uint64_t) isr20, 0x08, 0x00, 0x8E);
        setentry(21, (uint64_t) isr21, 0x08, 0x00, 0x8E);
        setentry(22, (uint64_t) isr22, 0x08, 0x00, 0x8E);
        setentry(23, (uint64_t) isr23, 0x08, 0x00, 0x8E);
        setentry(24, (uint64_t) isr24, 0x08, 0x00, 0x8E);
        setentry(25, (uint64_t) isr25, 0x08, 0x00, 0x8E);
        setentry(26, (uint64_t) isr26, 0x08, 0x00, 0x8E);
        setentry(27, (uint64_t) isr27, 0x08, 0x00, 0x8E);
        setentry(28, (uint64_t) isr28, 0x08, 0x00, 0x8E);
        setentry(29, (uint64_t) isr29, 0x08, 0x00, 0x8E);
        setentry(30, (uint64_t) isr30, 0x08, 0x00, 0x8E);
        setentry(31, (uint64_t) isr31, 0x08, 0x00, 0x8E);
        setentry(32, (uint64_t) irq0, 0x08, 0x00, 0x8E);
        setentry(33, (uint64_t) irq1, 0x08, 0x00, 0x8E);
        setentry(34, (uint64_t) irq2, 0x08, 0x00, 0x8E);
        setentry(35, (uint64_t) irq3, 0x08, 0x00, 0x8E);
        setentry(36, (uint64_t) irq4, 0x08, 0x00, 0x8E);
        setentry(37, (uint64_t) irq5, 0x08, 0x00, 0x8E);
        setentry(38, (uint64_t) irq6, 0x08, 0x00, 0x8E);
        setentry(39, (uint64_t) irq7, 0x08, 0x00, 0x8E);
        setentry(40, (uint64_t) irq8, 0x08, 0x00, 0x8E);
        setentry(41, (uint64_t) irq9, 0x08, 0x00, 0x8E);
        setentry(42, (uint64_t) irq10, 0x08, 0x00, 0x8E);
        setentry(43, (uint64_t) irq11, 0x08, 0x00, 0x8E);
        setentry(44, (uint64_t) irq12, 0x08, 0x00, 0x8E);
        setentry(45, (uint64_t) irq13, 0x08, 0x00, 0x8E);
        setentry(46, (uint64_t) irq14, 0x08, 0x00, 0x8E);
        setentry(47, (uint64_t) irq15, 0x08, 0x00, 0x8E);
        idtflush((uintptr_t) &pointer);
    }
}
