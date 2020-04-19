#include <timer.hpp>
#include <hwio.hpp>

namespace timer {
    uint64_t systicks = 0;

    void handler(idt::regs32_t *regs) {
        systicks++;
    }

    uint64_t sinceboot(timeunits_t unit) {
        switch(unit) {
            case TIMER_MILLISECONDS: return systicks; break;
            case TIMER_SECONDS: return systicks / 1000; break;
            case TIMER_MINUTES: return systicks / 60000; break;
        }

        return 0;
    }

    void sleep(uint64_t milliseconds) {
        uint64_t end = systicks + milliseconds;
        while(end > systicks) asm volatile("hlt");
    }

    void initpit(uint16_t frequency) {
        // Register handler and determine frequency divisor.
        uint16_t divisor = (uint16_t) (1193182 / frequency);
        idt::registerhandler(IRQ0, &handler);

        // Send init signal and divisor (split into two 8-bit messages) to PIT.
        outportb(0x43, 0x36);
        outportb(0x40, divisor & 0xFF);
        outportb(0x40, (divisor >> 8));
    }
}
