#include <gfx/renderer.hpp>
#include <mem/libc.hpp>
#include <string.hpp>
#include <errors.hpp>
#include <logger.hpp>
#include <timer.hpp>
#include <frogs.hpp>
#include <stdarg.h>

extern "C" {
    uintptr_t __stack_chk_guard = 0xC001BEEFDEADBEEF;

    __attribute__((noreturn)) void __stack_chk_fail(void) {
        panic("stack smashing guard overwritten!");
    }

    __attribute__((noreturn)) void panic(const char *format, ...) {
        // Disable interrupts.
        asm volatile("cli");

        // Setup the graphical renderer for the white death.
        memset(gfx::mdata.buffer, 0xFF, gfx::mdata.width * gfx::mdata.height * gfx::mdata.bpp);
        gfx::colour = gfx::column = 0;
        gfx::row = 1;

        gfx::write(errfrog);
        log::error("\n[kernel] freelsd panic: ");

        // Access the internal log vector table and print kernel panic to any that will accept.
        for(size_t i = 0; i < log::numwriters; i++) {
            if(log::writers[i].error) {
                va_list ap; va_start(ap, format);
                printk(log::writers[i].function, format, ap);
                va_end(ap);
            }
        }

        log::error("\n[kernel] halting execution. final system uptime: %ldms.\n", timer::sinceboot(TIMER_MILLISECONDS));

        // Enter an infinite loop.
        while(true) asm volatile("hlt");
    }
}
