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

mb_info_t *extractdata(mb_info_t *mbd) {
    // Copy the data from mbd into a new allocated block of memory.
    // We have 4K in a single block, using 120 bytes to copy mb_info_t.
    mb_info_t *mbdsafe = (mb_info_t*) physmem::allocblocks(1);
    memcpy(mbdsafe, mbd, sizeof(mb_info_t));

    // Copy the memory map into a safe memory area.
    // Total memory used here: 120 + mbd->mmaplength (estimate ~200 bytes)
    mb_mmap_t *mmapsafe = (mb_mmap_t*) mbdsafe + sizeof(mb_info_t);
    memcpy(mmapsafe, (void*) mbd->mmapaddr, mbd->mmaplength);
    mbdsafe->mmapaddr = (uint32_t) mmapsafe;

    return mbdsafe;
}

extern "C" void kernelmain(mb_info_t *mbd, uint32_t magic) {  
    gdt::initialise();
    idt::initialise();
    gfx::initialise(mbd);
    timer::initpit(1000);
    serial::initialise();
    kboard::initialise();
    physmem::initialise(mbd);
    virtmem::initialise();

    // Check if bootloader is multiboot-compliant.
    if(magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        serial::write("[kernel] not booted by a compliant bootloader.\n");
        panic("not booted by a compliant bootloader.");
    }

    // The FreeLSD frog!
    serial::write(stdfrog);
    gfx::write(stdfrog);

    // Create copies of mb_info_t and it's associated structs, we're about to wipe them out!
    mb_info_t *mbdsafe = extractdata(mbd);

    // Print the memory map to serial.
    mb_mmap_t *m = (mb_mmap_t*) mbdsafe->mmapaddr;
    while((uintptr_t) m < (uintptr_t) m + mbdsafe->mmaplength) {
        serial::printf("[physmm] memory map, region: 0x%p (start), 0x%p or %dKB (size), type %d\n", m->lowaddr, m->lowlen, m->lowlen / 1024, m->type);
        m = (mb_mmap_t*) ((uintptr_t) m + m->size + sizeof(m->size));
        if(m->type <= 0 || m->type >= 4) break;
    }

    // Write debugging information out to serial.
    serial::printf("\n[kernel] framebuffer address: 0x%p\n", gfx::data->buffer);
    serial::printf("[kernel] kernel end address: 0x%p\n", &kernelend);
    serial::printf("[kernel] resolution: %dx%dx%d\n", gfx::data->pixelwidth, gfx::data->pixelheight, gfx::data->bpp);

    // Write memory information to the screen.
    gfx::printf("[kernel] low memory: %dKB, high memory: %dKB\n", mbdsafe->lowermem, mbdsafe->uppermem);
    gfx::printf("[kernel] total memory available: %dKB\n", physmem::totalsize);

    // At this point, anything below 640K is fair game according to the physical memory manager.
    // Note for self: move anything useful below 640K out of the way before PMM decides to shit on it.
}
