#include <gfx/renderer.hpp>
#include <interrupts.hpp>
#include <multiboot.hpp>
#include <keyboard.hpp>
#include <mem/virt.hpp>
#include <mem/phys.hpp>
#include <string.hpp>
#include <serial.hpp>
#include <errors.hpp>
#include <timer.hpp>
#include <stdint.h>

extern "C" void kernelmain(uint64_t magic, uintptr_t mbaddr) {
    idt::initialise();
    mboot::initialise(mbaddr);
    serial::initialise();
    mem::initialisephys();
    mem::initialisevirt();
    timer::initpit(1000);
    kboard::initialise();
    gfx::initialise();

    // Check if bootloader is Multiboot2-compliant.
    if(magic != MULTIBOOT2_BOOTLOADER_MAGIC || mbaddr & 0x07) panic("bootloader is not multiboot2 compliant!");

    // Print the memory map out to serial.
    mb_mmap_entry_t *mmapcur = mboot::info.mmap->entries;
    while((uintptr_t) mmapcur < (uintptr_t) mboot::info.mmap->entries + mboot::info.mmap->size - 16) {
        klog("[physmm] memory map, %s | start: %p, length: %p or %ld KB\n", (mmapcur->type == MULTIBOOT_MEMORY_AVAILABLE) ? "AVAILABLE" : "RESERVED ", mmapcur->addr, mmapcur->len, mmapcur->len / 1024);
        mmapcur = (mb_mmap_entry_t*) ((uintptr_t) mmapcur + mboot::info.mmap->entrysize);
    } klog("\n");

    // Write some debugging information to serial.
    klog("[kernel] hello from long mode!\n");
    klog("[kernel] framebuffer address: %p\n", gfx::mdata.buffer);
    klog("[kernel] end-of-kernel address: %p\n", &kernelend);

    // The kernel cannot return, therefore we halt here.
    while(true) asm volatile("hlt");
}
