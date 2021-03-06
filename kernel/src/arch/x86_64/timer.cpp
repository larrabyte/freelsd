#include <timer.hpp>
#include <hwio.hpp>

namespace timer {
    uint64_t systicks = 0;

    void handler(idt::regs64_t *regs) {
        systicks++;
    }

    uint64_t sinceboot(timeunits_t unit) {
        switch(unit) {
            case TIMER_MILLISECONDS: return systicks;
            case TIMER_SECONDS: return systicks / 1000;
            case TIMER_MINUTES: return systicks / 60000;
        }

        return 0;
    }

    void sleep(uint64_t ms) {
        uint64_t end = systicks + ms;
        while(end > systicks) asm volatile("hlt");
    }

    void initpit(uint16_t frequency) {
        // Register interrupt handler and determine freq. divisor.
        uint16_t divisor = (uint16_t) (1193182 / frequency);
        idt::registerhandler(IRQ0, &handler);

        // Send init signal and divisor (split into two 8-bit messages) to the PIT.
        outportb(0x43, 0x36);
        outportb(0x40, divisor & 0xFF);
        outportb(0x40, (divisor >> 8));
    }
}
