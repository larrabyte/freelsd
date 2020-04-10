#include <timer.hpp>
#include <hwio.hpp>

namespace timer {
    uint64_t systicks = 0;

    void handler(idt::regs32_t *regs) {
        systicks++;
    }

    void sleep(uint64_t milliseconds) {
        uint64_t end = systicks + milliseconds;
        while(end > systicks) asm volatile("hlt");
    }

    void initpit(uint32_t frequency) {
        // Register handler and determine frequency divisor.
        uint32_t divisor = 1193180 / frequency;
        idt::registerhandler(IRQ0, &handler);

        // Send init signal and divisor (split into two 16-bit messages) to PIT.
        outportb(0x43, 0x36);
        outportb(0x40, divisor & 0xFF);
        outportb(0x40, (divisor >> 8) & 0xFF);
    }
}
