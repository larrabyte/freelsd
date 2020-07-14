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
#include <cpu.hpp>

extern "C" void kernelmain(uint64_t magic, uintptr_t mbaddr) {
    // Critical for early debugging.
    serial::initialise();
    mboot::initialise(magic, mbaddr);
    idt::initialise();
    timer::initpit(1000);

    // Memory and framebuffer renderer.
    mem::initialisephys();
    mem::initialisevirt();
    gfx::initialise();

    // Final initialisation procedures.
    kboard::initialise();
    cpu::initialise();
    log::initialise();

    // Print the memory map out to the kernel log.
    mb_mmap_entry_t *mmapcur = mboot::info.mmap->entries;
    while((uintptr_t) mmapcur < (uintptr_t) mboot::info.mmap->entries + mboot::info.mmap->size - 16) {
        log::info("[physmm] memory map, %s | start: %p, length: %p or %ld KB\n", mboot::getmmaptype(mmapcur->type), mmapcur->addr, mmapcur->len, mmapcur->len / 1024);
        mmapcur = (mb_mmap_entry_t*) ((uintptr_t) mmapcur + mboot::info.mmap->entrysize);
    } log::info("\n");

    // Write some debugging information to the log.
    log::info("[kernel] hello from long mode!\n");
    log::info("[kernel] framebuffer address: %p\n", gfx::mdata.buffer);
    log::info("[kernel] end-of-kernel address: %p\n\n", &kernelend);

    // Write information gathered from CPUID to the log.
    log::info("[kcpuid] CPU brand name: %s\n", cpu::getbrandname());
    log::info("[kcpuid] CPU vendor: %s\n", cpu::getvendor());
    log::info("[kcpuid] hypervisor: %s\n", cpu::supports(CPU_FEATURE_HVISOR) ? "true" : "false");

    // The kernel cannot return, therefore we halt here.
    while(true) asm volatile("hlt");
}
