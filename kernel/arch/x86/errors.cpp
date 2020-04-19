#include <gfx/renderer.hpp>
#include <mem/libc.hpp>
#include <string.hpp>
#include <errors.hpp>
#include <serial.hpp>
#include <timer.hpp>
#include <stdint.h>
#include <stdarg.h>

extern "C" {
    // Find out how to randomise this value for security.
    uintptr_t __stack_chk_guard = 0xDEADBEEF;

    __attribute__((noreturn)) void __stack_chk_fail(void) {
        panic("stack smashing guard overwritten.");
    }

    __attribute__((noreturn)) void panic(const char *format, ...) {
        // Disable interrupts.
        asm volatile("cli");

        // Setup the renderer for the white death.
        memset(gfx::data->buffer, 0xFF, gfx::data->pwidth * gfx::data->pheight * gfx::data->bpp);
        gfx::colour = 0; gfx::column = 0; gfx::row = 1;

        // Initialise variadic argument list.
        va_list ap; va_start(ap, format);

        // Print error to both serial and screen.
        serial::write("\n[kernel] freelsd panic: ");
        printk(&serial::writechar, format, ap);
        serial::printf("\n[kernel] halting execution. final system uptime: %dms.\n", timer::sinceboot(TIMER_MILLISECONDS));

        gfx::write(errfrog);
        gfx::write("[kernel] freelsd panic: ");
        printk(&gfx::writechar, format, ap);
        gfx::printf("\n[kernel] halting execution. final system uptime: %dms.\n", timer::sinceboot(TIMER_MILLISECONDS));

        // End argument list.
        va_end(ap);

        // Enter an infinite loop.
        while(true) asm volatile("hlt");
    }
}
