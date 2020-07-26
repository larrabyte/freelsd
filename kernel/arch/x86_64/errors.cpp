#include <gfx/renderer.hpp>
#include <interrupts.hpp>
#include <mem/libc.hpp>
#include <mem/phys.hpp>
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
    bool nestedpanic = false, stacksmashed = false;

    __attribute__((noreturn)) void __stack_chk_fail(void) {
        stacksmashed = true;
        panic("stack smashing guard overwritten!");
    }

    __attribute__((noreturn)) void internalpanic(const char *filename, const char *function, int line, idt::regs64_t *regs, const char *format, ...) {
        // Disable interrupts.
        asm volatile("cli");
        uint64_t *stackrbp, stackrip;

        // Setup the graphical renderer for the white death.
        memset(gfx::mdata.buffer, 0xFF, gfx::mdata.width * gfx::mdata.height * gfx::mdata.bpp);
        gfx::colour = gfx::column = 0;
        gfx::fill = 0xFF;
        gfx::row = 1;

        // Immediately halt if a new panic is nested.
        if(nestedpanic) {
            log::error("[kpanic] nested kernel panic!\n");
            log::error("[kpanic] system is in an unreliable state, halting.\n");
            goto halt;
        } else nestedpanic = true;

        gfx::printf("%s\n", errfrog);
        log::error("-------> FREELSD KERNEL PANIC!\n");
        log::error("-------> %s\n\n", panicmsgs[timer::systicks % (sizeof(panicmsgs) / sizeof(uintptr_t))]);
        log::error("[kpanic] freelsd panic in %s -> %s(), ln %d.\n", filename, function, line);
        log::error("[kpanic] reason: ");

        // Access the internal log vector table and print a kernel panic to any that will accept.
        for(size_t i = 0; i < log::numwriters; i++) {
            if(log::writers[i].error) {
                va_list ap; va_start(ap, format);
                printk(log::writers[i].function, format, ap);
                va_end(ap);
            }
        }

        // Log system uptime and physical memory usage.
        log::error("\n[kpanic] system uptime: %ld ticks, %d seconds.\n", timer::systicks, timer::sinceboot(TIMER_SECONDS));
        log::error("[kpanic] system memory usage: %zdMB/%zdMB.\n", (mem::usedblocks * PMMGR_BLOCK_SIZE) >> 20, mem::totalsize >> 20);

        // Print any available processor context.
        if(regs != nullptr) {
            log::error("\n[kpanic] processor context available.\n");
            log::error("[kpanic] rax: %p, rbx: %p, rcx: %p, rdx: %p\n", regs->rax, regs->rbx, regs->rcx, regs->rdx);
            log::error("[kpanic] rsi: %p, rdi: %p, rbp: %p, rsp: %p\n", regs->rsi, regs->rdi, regs->rbp, regs->rsp);
            log::error("[kpanic]  r8: %p,  r9: %p, r10: %p, r11: %p\n", regs->r8, regs->r9, regs->r10, regs->r11);
            log::error("[kpanic] r12: %p, r13: %p, r14: %p, r15: %p\n", regs->r12, regs->r13, regs->r14, regs->r15);
            log::error("[kpanic]  cs: %p,  ss: %p, rip: %p, rfl: %p\n\n", regs->cs, regs->ss, regs->rip, regs->rflags);
        }

        // Check if the stack was smashed before printing a stack trace.
        if(stacksmashed) {
            log::error("[kpanic] stack trace unavailable.\n");
            log::error("[kpanic] stack checking guard overwritten.\n");
            goto halt;
        }

        stackrbp = (uint64_t*) regs->rbp;
        log::error("[kpanic] stack trace available.\n", stackrbp);
        log::error("[kpanic] kernel panic occurred at %p.\n", regs->rip);

        while(stackrbp) {
            stackrip = *(stackrbp + 1);
            log::error("[kpanic] which was called from -> %p.\n", stackrip);
            stackrbp = (uint64_t*) (*stackrbp);
        }

        // Enter an infinite loop.
        halt: while(true) asm volatile("hlt");
    }
}
