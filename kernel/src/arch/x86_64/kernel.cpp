#include <gfx/renderer.hpp>
#include <interrupts.hpp>
#include <multiboot.hpp>
#include <keyboard.hpp>
#include <mem/virt.hpp>
#include <mem/phys.hpp>
#include <serial.hpp>
#include <logger.hpp>
#include <timer.hpp>
#include <acpi.hpp>
#include <apic.hpp>
#include <cpu.hpp>

// The end of the kernel.
extern "C" void *kernelend;

extern "C" __attribute__((noreturn)) void kernelmain(uint64_t magic, uintptr_t mbaddr) {
    // Critical for early debugging.
    serial::initialise();
    log::initialise();
    mboot::initialise(magic, mbaddr);

    // Interrupt initialisation.
    idt::initialise();
    timer::initpit(1000);
    kboard::initialise();

    // Memory initialisation.
    mem::initialisephys();
    mem::initialisevirt();
    gfx::initialise();

    // APIC/SMP initialisation.
    acpi::initialise();
    apic::initialisebsp();
    cpu::initialisestats();

    // Write some debugging information to the log.
    log::info("[kernel] hello from long mode!\n");
    log::info("[kernel] framebuffer address: %p\n", gfx::mdata.buffer);
    log::info("[kernel] kernel address endp: %p\n\n", &kernelend);

    // Write information gathered from CPUID to the log.
    log::info("[kcpuid] CPU brand name: %s\n", cpu::getbrandname());
    log::info("[kcpuid] CPU vendor: %s\n", cpu::getvendor());
    log::info("[kcpuid] hypervisor: %s\n", cpu::supports(CPU_FEATURE_HVISOR) ? "true" : "false");

    // The kernel cannot return, therefore we halt here.
    while(true) asm volatile("hlt");
}
