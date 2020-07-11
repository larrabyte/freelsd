#include <gfx/renderer.hpp>
#include <interrupts.hpp>
#include <multiboot.hpp>
#include <keyboard.hpp>
#include <mem/virt.hpp>
#include <mem/phys.hpp>
#include <string.hpp>
#include <serial.hpp>
#include <logger.hpp>
#include <errors.hpp>
#include <timer.hpp>
#include <stdint.h>

extern "C" void kernelmain(uint64_t magic, uintptr_t mbaddr) {
    mboot::initialise(mbaddr);
    mem::initialisephys();
    mem::initialisevirt();
    serial::initialise();
    idt::initialise();
    timer::initpit(1000);
    kboard::initialise();
    gfx::initialise();
    log::initialise();

    // Check if bootloader is Multiboot2-compliant.
    if(magic != MULTIBOOT2_BOOTLOADER_MAGIC || mbaddr & 0x07) panic("bootloader is not multiboot2 compliant!");

    // Print the memory map out to the kernel log.
    mb_mmap_entry_t *mmapcur = mboot::info.mmap->entries;
    while((uintptr_t) mmapcur < (uintptr_t) mboot::info.mmap->entries + mboot::info.mmap->size - 16) {
        log::info("[physmm] memory map, %s | start: %p, length: %p or %ld KB\n", (mmapcur->type == MULTIBOOT_MEMORY_AVAILABLE) ? "AVAILABLE" : "RESERVED ", mmapcur->addr, mmapcur->len, mmapcur->len / 1024);
        mmapcur = (mb_mmap_entry_t*) ((uintptr_t) mmapcur + mboot::info.mmap->entrysize);
    } log::info("\n");

    // Write some debugging information to the log.
    log::info("[kernel] hello from long mode!\n");
    log::info("[kernel] framebuffer address: %p\n", gfx::mdata.buffer);
    log::info("[kernel] end-of-kernel address: %p\n", &kernelend);

    // The kernel cannot return, therefore we halt here.
    while(true) asm volatile("hlt");
}
