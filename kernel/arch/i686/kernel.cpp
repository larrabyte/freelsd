#include <interrupts.hpp>
#include <multiboot.hpp>
#include <keyboard.hpp>
#include <gfx/gfx.hpp>
#include <memory.hpp>
#include <serial.hpp>
#include <string.hpp>
#include <timer.hpp>
#include <gdt.hpp>

static const char frog[] = "\n  ()-()\n.-(___)-. freelsd development kernel\n _<   >_  beep boop keeping track of time\n \\/   \\/\n\n";

void lighthouse(mb_info_t *mbd) {
    gfx::video_info_t *vinf = gfx::infoptr;
    uint8_t pixeldata = 0;

    while(true) {
        while(pixeldata < 255) memset(vinf->buffer, pixeldata++, vinf->pixelheight * vinf->pixelwidth * (vinf->bpp / 8));
        while(pixeldata > 000) memset(vinf->buffer, pixeldata--, vinf->pixelheight * vinf->pixelwidth * (vinf->bpp / 8));
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
        gfx::printf("[kernel] available memory: %dMB\n", mempages);
    }

    // Write VESA video mode information to serial.
    serial::printf("[kernel] framebuffer address: %p\n", gfx::info.buffer);
    serial::printf("[kernel] resolution: %dx%dx%d\n", gfx::info.pixelwidth, gfx::info.pixelheight, gfx::info.bpp);

    // lighthouse(mbd);
}
