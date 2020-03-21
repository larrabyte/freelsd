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
    // Cache framebuffer address and bytes per pixel.
    void *screen = (void*) mbd->framebufferaddr;
    uint8_t bpp = mbd->framebufferbpp / 8;
    uint8_t pixeldata = 0;

    while(true) {
        while(pixeldata < 255) memory::set(screen, pixeldata++, mbd->framebufferwidth * mbd->framebufferheight * bpp);
        while(pixeldata > 0)   memory::set(screen, pixeldata--, mbd->framebufferwidth * mbd->framebufferheight * bpp);
    }
}

extern "C" {
    void kernelmain(mb_info_t *mbd, uint32_t magic) {
        gdt::initialise();
        idt::initialise();
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

        // Write framebuffer address to serial.
        serial::write("[kernel] framebuffer address: ");
        serial::write(cstr::itoa(mbd->framebufferaddr, numascii, 10));
        serial::write("\n");

        // Start the lightshow!
        lighthouse(mbd);
    }
}