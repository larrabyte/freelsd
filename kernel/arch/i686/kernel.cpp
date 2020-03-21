#include <interrupts.hpp>
#include <multiboot.hpp>
#include <keyboard.hpp>
#include <memory.hpp>
#include <serial.hpp>
#include <timer.hpp>
#include <cstr.hpp>
#include <gdt.hpp>
#include <gfx.hpp>

void lighthouse(mb_info_t *mbd) {
    uint32_t height = mbd->framebufferheight;
    uint32_t width = mbd->framebufferwidth;
    uint32_t bpp = mbd->framebufferbpp / 8;
    uint8_t pixeldata = 0;

    while(true) {
        while(pixeldata < 255) memory::set(gfx::buffer, pixeldata++, height * width * bpp);
        while(pixeldata > 000) memory::set(gfx::buffer, pixeldata--, height * width * bpp);
    }
}

extern "C" {
    void kernelmain(mb_info_t *mbd, uint32_t magic) {
        gdt::initialise();
        idt::initialise();
        gfx::initialise(mbd);
        timer::initpit(1000);
        serial::initialise();
        kboard::initialise();
        char numascii[20];

        // The FreeLSD frog!
        serial::write("\n  ()-()\n.-(___)-. freelsd development kernel\n _<   >_  beep boop keeping track of time\n \\/   \\/\n\n");

        if(checkbit(mbd->flags, 0)) {
            // Read number of memory blocks and divide to retrieve megabytes.
            uint64_t mempages = (mbd->lowermem + mbd->uppermem) / 1024;

            // Write available memory to serial.
            serial::write("[kernel] available memory: ");
            serial::write(cstr::itoa(mempages, numascii, 10));
            serial::write("MB\n");
        }

        // Write VESA video mode information to serial.
        serial::write("[kernel] framebuffer address: ");
        serial::write(cstr::itoa(mbd->framebufferaddr, numascii, 10));
        serial::write("\n");
        serial::write("[kernel] resolution: ");
        serial::write(cstr::itoa(mbd->framebufferwidth, numascii, 10));
        serial::write("x");
        serial::write(cstr::itoa(mbd->framebufferheight, numascii, 10));
        serial::write("x");
        serial::write(cstr::itoa(mbd->framebufferbpp, numascii, 10));
        serial::write("\n");

        // Start the lightshow!
        lighthouse(mbd);
    }
}