#include <gfx/renderer.hpp>
#include <multiboot.hpp>
#include <keyboard.hpp>
#include <mem/virt.hpp>
#include <mem/phys.hpp>
#include <serial.hpp>
#include <errors.hpp>
#include <cpuid.hpp>
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
        serial::printf("[physmm] memory map, region: %p (start), %p or %dKB (size), type %d\n", m->lowaddr, m->lowlen, m->lowlen / 1024, m->type);
        m = (mb_mmap_t*) ((uintptr_t) m + m->size + sizeof(m->size));
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
    gfx::printf("[kernel] CPUID | vendor string: %s\n", cpuinfo.vendor);
    gfx::printf("[kernel] CPUID | SSE1 support: %s\n", (cpuinfo.edx & CPUID_FEATURE_EDX_SSE) ? "yes" : "no");
    gfx::printf("[kernel] CPUID | SSE2 support: %s\n", (cpuinfo.edx & CPUID_FEATURE_EDX_SSE2) ? "yes" : "no");
    gfx::printf("[kernel] CPUID | SSE3 support: %s\n", (cpuinfo.ecx & CPUID_FEATURE_ECX_SSE3) ? "yes" : "no");
    gfx::printf("[kernel] CPUID | SSSE3 support: %s\n", (cpuinfo.ecx & CPUID_FEATURE_ECX_SSSE3) ? "yes" : "no");
    gfx::printf("[kernel] CPUID | SSE41 support: %s\n", (cpuinfo.ecx & CPUID_FEATURE_ECX_SSE41) ? "yes" : "no");
    gfx::printf("[kernel] CPUID | SSE42 support: %s\n", (cpuinfo.ecx & CPUID_FEATURE_ECX_SSE42) ? "yes" : "no");

    // Infinite loop here, we never return.
    while(true) asm volatile("hlt");
}
