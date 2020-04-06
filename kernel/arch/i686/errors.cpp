#include <gfx/renderer.hpp>
#include <errors.hpp>
#include <memory.hpp>
#include <serial.hpp>
#include <stdint.h>

__attribute__((noreturn)) void panic(const char *message) {
    // Disable interrupts.
    asm volatile("cli");

    // Set the framebuffer to white.
    memset(gfx::info.buffer, 0xFF, gfx::info.pixelwidth * gfx::info.pixelheight * (gfx::info.bpp / 8));

    // Reset gfx colour and cursor.
    gfx::colour = 0x00000000;
    gfx::column = 0;
    gfx::row = 1;

    gfx::printf(errfrog);
    gfx::printf("[kernel] freelsd panic: %s\n", message);
    gfx::printf("[kernel] halting execution.\n");

    while(true) asm volatile("hlt");
}

extern "C" {
    // Find out how to randomise this value for security.
    uintptr_t __stack_chk_guard = 0xDEADBEEF;

    __attribute__((noreturn)) void __stack_chk_fail(void) {
        panic("Stack smashing guard overwritten.");
    }
}
