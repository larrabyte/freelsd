#include <memory.hpp>
#include <hwio.hpp>
#include <cstr.hpp>
#include <idt.hpp>
#include <vga.hpp>

idt::handler_t handlers[IDTSIZE];
idt::entry_t idtarray[IDTSIZE];
idt::ptr_t idtptr;

extern "C" {
    void irqhandler(idt::registers_t *regs) {
        if(regs->intnum >= 40) outportb(0xA0, 0x20);
        outportb(0x20, 0x20);

        if(handlers[regs->intnum]) {
            idt::handler_t handle = handlers[regs->intnum];
            handle(*regs);
        }
    }

    void isrhandler(idt::registers_t *regs) {
        char asciinum[20];

        if(handlers[regs->intnum]) {
            idt::handler_t handle = handlers[regs->intnum];
            handle(*regs);
        } else {
            vga::write("[isr] unhandled interrupt: ");
            vga::write(cstr::itoa(regs->intnum, asciinum, 10));
            vga::write("\n");
        }
    }

    void idtflush(void);
}

static void setgate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags) {
    idtarray[num].basehigh = (base >> 16) & 0xFFFF;
    idtarray[num].baselow = base & 0xFFFF;
    idtarray[num].selector = selector;
    idtarray[num].flags = flags;
    idtarray[num].zero = 0x0;
}

void idt::registerhandler(uint8_t interrupt, handler_t function) {
    handlers[interrupt] = function;
}

void idt::initialise(void) {
    idtptr.limit = sizeof(idt::entry_t) * IDTSIZE - 1;
    idtptr.base = (uint32_t) &idtarray;

    memory::set(handlers, 0, sizeof(idt::handler_t) * IDTSIZE);
    memory::set(idtarray, 0, sizeof(idt::entry_t) * IDTSIZE);

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

    setgate(0, (uint32_t) isr0, 0x08, 0x8E);
    setgate(1, (uint32_t) isr1, 0x08, 0x8E);
    setgate(2, (uint32_t) isr2, 0x08, 0x8E);
    setgate(3, (uint32_t) isr3, 0x08, 0x8E);
    setgate(4, (uint32_t) isr4, 0x08, 0x8E);
    setgate(5, (uint32_t) isr5, 0x08, 0x8E);
    setgate(6, (uint32_t) isr6, 0x08, 0x8E);
    setgate(7, (uint32_t) isr7, 0x08, 0x8E);
    setgate(8, (uint32_t) isr8, 0x08, 0x8E);
    setgate(9, (uint32_t) isr9, 0x08, 0x8E);
    setgate(10, (uint32_t) isr10, 0x08, 0x8E);
    setgate(11, (uint32_t) isr11, 0x08, 0x8E);
    setgate(12, (uint32_t) isr12, 0x08, 0x8E);
    setgate(13, (uint32_t) isr13, 0x08, 0x8E);
    setgate(14, (uint32_t) isr14, 0x08, 0x8E);
    setgate(15, (uint32_t) isr15, 0x08, 0x8E);
    setgate(16, (uint32_t) isr16, 0x08, 0x8E);
    setgate(17, (uint32_t) isr17, 0x08, 0x8E);
    setgate(18, (uint32_t) isr18, 0x08, 0x8E);
    setgate(19, (uint32_t) isr19, 0x08, 0x8E);
    setgate(20, (uint32_t) isr20, 0x08, 0x8E);
    setgate(21, (uint32_t) isr21, 0x08, 0x8E);
    setgate(22, (uint32_t) isr22, 0x08, 0x8E);
    setgate(23, (uint32_t) isr23, 0x08, 0x8E);
    setgate(24, (uint32_t) isr24, 0x08, 0x8E);
    setgate(25, (uint32_t) isr25, 0x08, 0x8E);
    setgate(26, (uint32_t) isr26, 0x08, 0x8E);
    setgate(27, (uint32_t) isr27, 0x08, 0x8E);
    setgate(28, (uint32_t) isr28, 0x08, 0x8E);
    setgate(29, (uint32_t) isr29, 0x08, 0x8E);
    setgate(30, (uint32_t) isr30, 0x08, 0x8E);
    setgate(31, (uint32_t) isr31, 0x08, 0x8E);
    setgate(32, (uint32_t) irq0, 0x08, 0x8E);
    setgate(33, (uint32_t) irq1, 0x08, 0x8E);
    setgate(34, (uint32_t) irq2, 0x08, 0x8E);
    setgate(35, (uint32_t) irq3, 0x08, 0x8E);
    setgate(36, (uint32_t) irq4, 0x08, 0x8E);
    setgate(37, (uint32_t) irq5, 0x08, 0x8E);
    setgate(38, (uint32_t) irq6, 0x08, 0x8E);
    setgate(39, (uint32_t) irq7, 0x08, 0x8E);
    setgate(40, (uint32_t) irq8, 0x08, 0x8E);
    setgate(41, (uint32_t) irq9, 0x08, 0x8E);
    setgate(42, (uint32_t) irq10, 0x08, 0x8E);
    setgate(43, (uint32_t) irq11, 0x08, 0x8E);
    setgate(44, (uint32_t) irq12, 0x08, 0x8E);
    setgate(45, (uint32_t) irq13, 0x08, 0x8E);
    setgate(46, (uint32_t) irq14, 0x08, 0x8E);
    setgate(47, (uint32_t) irq15, 0x08, 0x8E);
    idtflush();
}