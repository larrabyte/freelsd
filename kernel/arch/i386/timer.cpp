#include <timer.hpp>
#include <cstr.hpp>
#include <hwio.hpp>
#include <idt.hpp>
#include <vga.hpp>

uint32_t tick = 0;

void timercallback(idt::registers_t regs) {
    char numascii[20];

    vga::write("[pit] Received int. ");
    vga::write(cstr::itoa(regs.intnum, numascii, 10));
    vga::write(", system is on tick #");
    vga::write(cstr::itoa(tick++, numascii, 10));
    vga::write("\n");
}

void timer::initpit(uint32_t frequency) {
    // Register handler and determine frequency divisor.
    idt::registerhandler(IRQ0, &timercallback);
    uint32_t divisor = 1193180 / frequency;

    // Send init signal and divisor (split into two 8-bit messages) to PIT.
    outportb(0x43, 0x36);
    outportb(0x40, divisor & 0xFF);
    outportb(0x40, (divisor >> 8) & 0xFF);
}