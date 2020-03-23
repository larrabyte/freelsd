#include <interrupts.hpp>
#include <multiboot.hpp>
#include <keyboard.hpp>
#include <gfx/gfx.hpp>
#include <memory.hpp>
#include <serial.hpp>
#include <timer.hpp>
#include <cstr.hpp>
#include <gdt.hpp>

static const char frog[] = "\n  ()-()\n.-(___)-. freelsd development kernel\n _<   >_  beep boop keeping track of time\n \\/   \\/\n\n";

void lighthouse(mb_info_t *mbd) {
    uint32_t height = mbd->framebufferheight;
    uint32_t width = mbd->framebufferwidth;
    uint32_t bpp = mbd->framebufferbpp / 8;
    uint8_t pixeldata = 0;

    while(true) {
        while(pixeldata < 255) memory::set(gfx::info.buffer, pixeldata++, height * width * bpp);
        while(pixeldata > 000) memory::set(gfx::info.buffer, pixeldata--, height * width * bpp);
    }
}

extern "C" void kernelmain(mb_info_t *mbd, uint32_t magic) {
    gdt::initialise();
    idt::initialise();
    gfx::initialise(mbd);
    timer::initpit(1000);
    serial::initialise();
    kboard::initialise();

    // The FreeLSD frog!
    serial::write(frog);
    gfx::write(frog);

    if(checkbit(mbd->flags, 0)) {
        // Read number of memory blocks and divide to retrieve megabytes.
        uint64_t mempages = (mbd->lowermem + mbd->uppermem) / 1024;

        // Write available memory to screen.
        gfx::write("[kernel] available memory: ");
        gfx::write(cstr::itoa(mempages, 10));
        gfx::write("MB\n");
    }

    // Write VESA video mode information to serial.
    serial::write("[kernel] framebuffer address: ");
    serial::write(cstr::itoa(mbd->framebufferaddr, 10));
    serial::write("\n");
    serial::write("[kernel] resolution: ");
    serial::write(cstr::itoa(mbd->framebufferwidth, 10));
    serial::write("x");
    serial::write(cstr::itoa(mbd->framebufferheight, 10));
    serial::write("x");
    serial::write(cstr::itoa(mbd->framebufferbpp, 10));
    serial::write("\n");
}