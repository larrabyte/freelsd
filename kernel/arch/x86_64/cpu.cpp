#include <mem/libc.hpp>
#include <errors.hpp>
#include <cpu.hpp>

namespace cpu {
    static const char *exceptionstr[] = {
        "divide-by-zero exception.",
        "debug exception.",
        "non-maskable interrupt.",
        "breakpoint exception.",
        "overflow exception.",
        "bound range exceeded.",
        "invalid opcode exception.",
        "device not available.",
        "double fault exception.",
        "reserved exception.",
        "invalid task state segment.",
        "segment not present.",
        "stack segment fault.",
        "general protection fault.",
        "page fault exception.",
        "reserved exception.",
        "x87 floating-point exception.",
        "alignment check exception.",
        "machine check exception.",
        "SIMD floating-point exception.",
        "virtualisation exception.",
        "reserved exception.",
        "reserved exception.",
        "reserved exception.",
        "reserved exception.",
        "reserved exception.",
        "reserved exception.",
        "reserved exception.",
        "reserved exception.",
        "reserved exception.",
        "security exception.",
        "reserved exception."
    };

    static cpuid_regs_t registers;
    static cpuid_stats_t stats;

    void handler(idt::regs64_t *regs) {
        // Call kernel panic w/ interrupt context.
        ctxpanic(regs, exceptionstr[regs->isr]);
    }

    char *getvendor(void) {
        // Return vendor from stats.
        return stats.vendor;
    }

    char *getbrandname(void) {
        // Return brand name from stats.
        return stats.brand;
    }

    bool supports(cpuid_feature_t feature) {
        // Execute CPUID with a leaf of 1.
        if(stats.stdmax >= 1) readcpuid(1, &registers);
        else panic("CPUID leaf 0x1 not supported!");

        // Return a boolean depending on whether the feature exists in the specified register.
        if(feature & CPUID_EDX_BIT) return (registers.edx & (feature ^ CPUID_EDX_BIT)) ? true : false;
        else return (registers.ecx & feature) ? true : false;
    }

    void initialisestats(void) {
        // Get highest standard function.
        readcpuid(0, &registers);
        stats.stdmax = registers.eax;

        // Copy vendor string into stats.
        memcpy(stats.vendor + 0, &registers.ebx, 4);
        memcpy(stats.vendor + 4, &registers.edx, 4);
        memcpy(stats.vendor + 8, &registers.ecx, 4);
        stats.vendor[12] = '\0';

        // Get highest extended function.
        readcpuid(0x80000000, &registers);
        stats.extmax = registers.eax;

        // If possible, store processor brand string.
        if(stats.extmax < 0x80000004) panic("CPUID leaf 0x80000004 not supported!");
        readcpuid(0x80000002, &stats.brand);
        readcpuid(0x80000003, &stats.brand[16]);
        readcpuid(0x80000004, &stats.brand[32]);
    }
}
