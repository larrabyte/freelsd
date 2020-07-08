#include <gfx/renderer.hpp>
#include <mem/libc.hpp>
#include <serial.hpp>
#include <string.hpp>
#include <errors.hpp>
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

        // Setup the renderer for the white death.
        memset(gfx::mdata.buffer, 0xFF, gfx::mdata.width * gfx::mdata.height * gfx::mdata.bpp);
        gfx::colour = gfx::column = 0;
        gfx::row = 1;

        // Initialise variadic argument list.
        va_list ap; va_start(ap, format);

        // Print error to both serial and screen.
        serial::write("\n[kernel] freelsd panic: ");
        printk(&serial::writechar, format, ap);
        serial::write("\n[kernel] halting execution. final system uptime: NANms.\n");

        gfx::write(errfrog);
        gfx::write("[kernel] freelsd panic: ");
        printk(&gfx::writechar, format, ap);
        gfx::write("\n[kernel] halting execution. final system uptime: NANms.\n");

        // End argument list.
        va_end(ap);

        // Enter an infinite loop.
        while(true) asm volatile("hlt");
    }
}
