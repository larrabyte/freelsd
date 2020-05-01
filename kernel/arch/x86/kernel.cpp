#include <gfx/renderer.hpp>
#include <multiboot.hpp>
#include <mem/alloc.hpp>
#include <keyboard.hpp>
#include <mem/virt.hpp>
#include <mem/phys.hpp>
#include <serial.hpp>
#include <errors.hpp>
#include <initrd.hpp>
#include <string.hpp>
#include <cpuid.hpp>
#include <timer.hpp>
#include <vfs.hpp>
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

    // Initialise the initial ramdisk.
    mb_modlist_t *rdinfo = (mb_modlist_t*) mbd->modaddr;
    if(rdinfo != 0) initrd::initialise(rdinfo->modstart);

    // Print the memory map to serial.
    mb_mmap_t *mmap = (mb_mmap_t*) mbd->mmapaddr;
    while((uintptr_t) mmap < mbd->mmapaddr + mbd->mmaplength) {
        serial::printf("[physmm] memory map, region: %p (start), %p or %dKB (size), type %d\n", mmap->lowaddr, mmap->lowlen, mmap->lowlen / 1024, mmap->type);
        mmap = (mb_mmap_t*) ((uintptr_t) mmap + mmap->size + sizeof(mmap->size));
    }

    // Write debugging information out to serial.
    serial::printf("\n[kernel] framebuffer address: %p\n", gfx::data->buffer);
    serial::printf("[kernel] kernel end address: %p\n", &kernelend);
    serial::printf("[kernel] resolution: %dx%dx%d\n", gfx::data->pwidth, gfx::data->pheight, gfx::data->bpp * 8);

    // Write memory information to the screen.
    gfx::printf("[kernel] low memory: %dKB, high memory: %dKB\n", mbd->lowermem, mbd->uppermem);
    gfx::printf("[kernel] total memory available: %dKB\n\n", physmem::totalsize);

    // Write CPUID information to the screen.
    cpuid_info_t cpuinfo = cpu::executecpuid();
    gfx::printf("[kernel] CPU vendor string: %s\n", cpuinfo.vendor);
    gfx::printf("[kernel] CPU SSE1 support: %s\n", (cpuinfo.edx & CPUID_FEATURE_EDX_SSE) ? "yes" : "no");
    gfx::printf("[kernel] CPU SSE2 support: %s\n", (cpuinfo.edx & CPUID_FEATURE_EDX_SSE2) ? "yes" : "no");
    gfx::printf("[kernel] CPU SSE3 support: %s\n", (cpuinfo.ecx & CPUID_FEATURE_ECX_SSE3) ? "yes" : "no");
    gfx::printf("[kernel] CPU SSSE3 support: %s\n", (cpuinfo.ecx & CPUID_FEATURE_ECX_SSSE3) ? "yes" : "no");
    gfx::printf("[kernel] CPU SSE41 support: %s\n", (cpuinfo.ecx & CPUID_FEATURE_ECX_SSE41) ? "yes" : "no");
    gfx::printf("[kernel] CPU SSE42 support: %s\n", (cpuinfo.ecx & CPUID_FEATURE_ECX_SSE42) ? "yes" : "no");

    vfs::dirent_t *ent;
    for(int i = 0; i < initrd::nodecount; i++) {
        // Iterate through each file in the directory.
        if((ent = vfs::readdir(initrd::root, i)) != 0) {
            // Check if it's name is equal to "lts128px.bmp".
            if(strcmp(ent->name, "lts128px.bmp") == 0) {
                // If so, find the node associated with it and copy it into memory.
                vfs::node_t *node = vfs::finddir(initrd::root, ent->name);
                uint8_t *fbuf = (uint8_t*) kmalloc(node->length);
                vfs::read(node, 0, node->length, fbuf);

                // Draw the bitmap and then free the buffer.
                gfx::drawbmp(fbuf, gfx::data->pwidth * 0.6, gfx::data->pheight * 0.05);
                kfree(fbuf);
            }
        }
    }

    // Infinite loop here, we never return.
    while(true) asm volatile("hlt");
}
