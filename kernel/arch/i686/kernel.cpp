#include <mem/physalloc.hpp>
#include <gfx/renderer.hpp>
#include <interrupts.hpp>
#include <multiboot.hpp>
#include <keyboard.hpp>
#include <mem/libc.hpp>
#include <serial.hpp>
#include <string.hpp>
#include <errors.hpp>
#include <timer.hpp>
#include <gdt.hpp>

void lighthouse(mb_info_t *mbd) {
    gfx::video_info_t *vinf = gfx::infoptr;
    uint8_t pixeldata = 0;

    while(true) {
        while(pixeldata < 255) memset(vinf->buffer, pixeldata++, vinf->pixelheight * vinf->pixelwidth * (vinf->bpp / 8));
        while(pixeldata > 000) memset(vinf->buffer, pixeldata--, vinf->pixelheight * vinf->pixelwidth * (vinf->bpp / 8));
    }
}

extern "C" void *kernelend;

extern "C" void kernelmain(mb_info_t *mbd, uint32_t magic) {
    gdt::initialise();
    idt::initialise();
    gfx::initialise(mbd);
    timer::initpit(1000);
    serial::initialise();
    kboard::initialise();

    // Check if bootloader is multiboot-compliant.
    if(magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        serial::write("[kernel] not booted by a compliant bootloader.\n");
        panic("not booted by a compliant bootloader.");
    }

    // The FreeLSD frog!
    serial::write(stdfrog);
    gfx::write(stdfrog);

    // Write VESA video mode information to serial.
    serial::printf("[kernel] framebuffer address: 0x%p\n", gfx::info.buffer);
    serial::printf("[kernel] resolution: %dx%dx%d\n", gfx::info.pixelwidth, gfx::info.pixelheight, gfx::info.bpp);
    serial::printf("[kernel] end of kernel: 0x%p\n", &kernelend);

    // Initialise PMM.
    physmem::initialise(mbd);
    gfx::printf("[kernel] total memory available: %d KB\n\n", physmem::totalsize);

    // lighthouse(mbd);
}
