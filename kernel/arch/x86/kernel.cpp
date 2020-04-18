#include <gfx/renderer.hpp>
#include <multiboot.hpp>
#include <keyboard.hpp>
#include <mem/libc.hpp>
#include <mem/virt.hpp>
#include <mem/phys.hpp>
#include <serial.hpp>
#include <string.hpp>
#include <errors.hpp>
#include <timer.hpp>
#include <gdt.hpp>

extern "C" void kernelmain(mb_info_t *mbd) {
    gdt::initialise();
    idt::initialise();
    physmem::initialise(mbd);
    virtmem::initialise(mbd);
    gfx::initialise(mbd);
    timer::initpit(1000);
    serial::initialise();
    kboard::initialise();

    // Print the memory map to serial.
    mb_mmap_t *m = (mb_mmap_t*) mbd->mmapaddr;
    while((uintptr_t) m < mbd->mmapaddr + mbd->mmaplength) {
        serial::printf("[physmm] memory map, region: 0x%p (start), 0x%p or %dKB (size), type %d\n", m->lowaddr, m->lowlen, m->lowlen / 1024, m->type);
        m = (mb_mmap_t*) ((uintptr_t) m + m->size + sizeof(m->size));
    }

    // Write debugging information out to serial.
    serial::printf("\n[kernel] framebuffer address: 0x%p\n", gfx::data->buffer);
    serial::printf("[kernel] kernel end address: 0x%p\n", &kernelend);
    serial::printf("[kernel] resolution: %dx%dx%d\n", gfx::data->pwidth, gfx::data->pheight, gfx::data->bpp * 8);

    // Write memory information to the screen.
    gfx::printf("[kernel] low memory: %dKB, high memory: %dKB\n", mbd->lowermem, mbd->uppermem);
    gfx::printf("[kernel] total memory available: %dKB\n", physmem::totalsize);

    // Infinite loop here, we never return.
    while(true) asm volatile("hlt");
}
