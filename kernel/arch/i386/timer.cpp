#include <timer.hpp>
#include <cstr.hpp>
#include <hwio.hpp>
#include <idt.hpp>
#include <vga.hpp>

uint32_t tick = 0;

void tcallback(idt::registers_t regs) {
    char numascii[20];

    vga::write("[pit] Received int. ");
    vga::write(cstr::itoa(regs.intnum, numascii, 10));
    vga::write(", system is on tick #");
    vga::write(cstr::itoa(tick++, numascii, 10));
    vga::write("\n");
}

void timer::initpit(uint32_t frequency) {
    idt::registerhandler(IRQ0, &tcallback);
    uint32_t divisor = 1193180 / frequency;
    uint8_t l = (uint8_t) divisor & 0xFF;
    uint8_t h = (uint8_t) ((divisor >> 8) & 0xFF);

    outportb(0x43, 0x36);
    outportb(0x40, l);
    outportb(0x40, h);
}