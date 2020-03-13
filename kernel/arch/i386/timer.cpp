#include <timer.hpp>
#include <cstr.hpp>
#include <hwio.hpp>
#include <idt.hpp>
#include <vga.hpp>

int64_t systemtick = 0;

void timercallback(idt::registers_t regs) {
    systemtick++;
}

void timer::sleep(uint64_t milliseconds) {
    int64_t end = systemtick + milliseconds;
    while(end > systemtick) asm volatile("hlt");
}

void timer::initpit(uint32_t frequency) {
    // Register handler and determine frequency divisor.
    idt::registerhandler(IRQ0, &timercallback);
    uint32_t divisor = 1193180 / frequency;

    // Send init signal and divisor (split into two 16-bit messages) to PIT.
    outportb(0x43, 0x36);
    outportb(0x40, divisor & 0xFF);
    outportb(0x40, (divisor >> 8) & 0xFF);
}