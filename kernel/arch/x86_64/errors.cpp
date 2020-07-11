#include <gfx/renderer.hpp>
#include <interrupts.hpp>
#include <mem/libc.hpp>
#include <string.hpp>
#include <errors.hpp>
#include <logger.hpp>
#include <timer.hpp>
#include <frogs.hpp>
#include <stdarg.h>

static const char *panicmsgs[] = {
    "// #kernelpanic #swag",
    "// 0xC001BEEFDEADBEEF",
    "// ABI broke, please fix!",
    "// DON'T PANIC, NOTHING IS OKAY!",
    "// Get out of my room, I'm playing Minecraft!",
    "// help_the_underscores_have_taken_over",
    "// NULL",
    "// Typo in the code.",
    "// Surprise! Haha. Well, this is awkward.",
    "// Butterfingers.",
    "// https://github.com/larrabyte/freelsd",
    "// see: bodging",
    "// 1337",
    "// Anonymous has taken control of this computer.",
    "// funny_joke_t comedy = new funny_joke_t;",
    "// Made in Java",
    "// FreeLSD > FreeBSD",
    "// swagbucks",
    "// you got clapped m8",
    "// you broke the world record for crashing freelsd!"
};

extern "C" {
    uintptr_t __stack_chk_guard = 0xC001BEEFDEADBEEF;

    __attribute__((noreturn)) void __stack_chk_fail(void) {
        panic("stack smashing guard overwritten!");
    }

    __attribute__((noreturn)) void internalpanic(const char *filename, const char *function, int line, const char *format, ...) {
        // Disable interrupts.
        asm volatile("cli");

        // Setup the graphical renderer for the white death.
        memset(gfx::mdata.buffer, 0xFF, gfx::mdata.width * gfx::mdata.height * gfx::mdata.bpp);
        gfx::colour = gfx::column = 0;
        gfx::row = 1;

        gfx::write(errfrog);
        log::error("\n-------> FREELSD KERNEL PANIC!\n");
        log::error("         %s\n\n", panicmsgs[timer::systicks % (sizeof(panicmsgs) / sizeof(uintptr_t))]);
        log::error("[kernel] freelsd panic: ");

        // Access the internal log vector table and print a kernel panic to any that will accept.
        for(size_t i = 0; i < log::numwriters; i++) {
            if(log::writers[i].error) {
                va_list ap; va_start(ap, format);
                printk(log::writers[i].function, format, ap);
                va_end(ap);
            }
        }

        log::error("\n[kernel] at %s -> %s(), ln %d.\n", filename, function, line);
        log::error("[kernel] halting execution. final system uptime: %ldms.\n", timer::sinceboot(TIMER_MILLISECONDS));

        // Enter an infinite loop.
        while(true) {
            asm volatile("cli");
            asm volatile("hlt");
        }
    }
}
