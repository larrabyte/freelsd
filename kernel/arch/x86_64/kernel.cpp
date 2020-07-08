#include <gfx/renderer.hpp>
#include <interrupts.hpp>
#include <multiboot.hpp>
#include <mem/virt.hpp>
#include <mem/phys.hpp>
#include <mem/libc.hpp>
#include <serial.hpp>
#include <stdint.h>

extern "C" void kernelmain(uint64_t magic, uintptr_t mbaddr) {
    idt::initialise();
    serial::initialise();
    mboot::initialise(mbaddr);
    mem::initialisephys();
    mem::initialisevirt();
    gfx::initialise();

    // Check if bootloader is Multiboot2-compliant.
    if(magic != MULTIBOOT2_BOOTLOADER_MAGIC || mbaddr & 0x07) {
        serial::printf("[kernel] bootloader is not multiboot2 compliant!\n");
        serial::printf("[kernel] magic/mbi: 0x%lx/0x%lx\n", magic, mbaddr);
    }

    // Print the memory map out to serial.
    mb_mmap_entry_t *mmapcur = mboot::info.mmap->entries;
    while((uintptr_t) mmapcur < (uintptr_t) mboot::info.mmap->entries + mboot::info.mmap->size - 16) {
        serial::printf("[physmm] memory map, %s | start: %p, length: %p or %ld KB\n", (mmapcur->type == MULTIBOOT_MEMORY_AVAILABLE) ? "AVAILABLE" : "RESERVED ", mmapcur->addr, mmapcur->len, mmapcur->len / 1024);
        mmapcur = (mb_mmap_entry_t*) ((uintptr_t) mmapcur + mboot::info.mmap->entrysize);
    } serial::write("\n");

    // Write some debugging information to serial.
    serial::printf("[kernel] hello from long mode!\n");
    serial::printf("[kernel] framebuffer address: %p\n", gfx::mdata.buffer);
    serial::printf("[kernel] end-of-kernel address: %p\n", &kernelend);
}
