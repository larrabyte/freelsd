#include <gfx/renderer.hpp>
#include <mem/libc.hpp>
#include <serial.hpp>
#include <string.hpp>
#include <errors.hpp>
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

        // Setup the renderer for the white death.
        memset(gfx::mdata.buffer, 0xFF, gfx::mdata.width * gfx::mdata.height * gfx::mdata.bpp);
        gfx::colour = gfx::column = 0;
        gfx::row = 1;

        // Initialise variadic argument list.
        va_list apg, aps;
        va_start(apg, format);
        va_start(aps, format);

        // Write panic message to both the framebuffer and serial.
        gfx::write(errfrog);
        klog("\n[kernel] freelsd panic: ");
        klog(format, apg, aps);
        klog("\n[kernel] halting execution. final system uptime: %ldms.\n", timer::sinceboot(TIMER_MILLISECONDS));

        // End argument list.
        va_end(apg);
        va_end(aps);

        // Enter an infinite loop.
        while(true) asm volatile("hlt");
    }
}
