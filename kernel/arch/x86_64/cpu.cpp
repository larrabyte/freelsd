#include <mem/libc.hpp>
#include <errors.hpp>
#include <cpu.hpp>

namespace cpu {
    static const char *exceptionstr[] = {
        "Divide-by-zero exception.",
        "Debug exception.",
        "Non-maskable interrupt.",
        "Breakpoint exception.",
        "Overflow exception.",
        "Bound range exceeded.",
        "Invalid opcode exception.",
        "Device not available.",
        "Double fault exception.",
        "Reserved exception.",
        "Invalid task state segment.",
        "Segment not present.",
        "Stack segment fault.",
        "General protection fault.",
        "Page fault exception.",
        "Reserved exception.",
        "x87 floating-point exception.",
        "Alignment check exception.",
        "Machine check exception.",
        "SIMD floating-point exception.",
        "Virtualisation exception.",
        "Reserved exception.",
        "Reserved exception.",
        "Reserved exception.",
        "Reserved exception.",
        "Reserved exception.",
        "Reserved exception.",
        "Reserved exception.",
        "Reserved exception.",
        "Reserved exception.",
        "Security exception.",
        "Reserved exception."
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

    void initialise(void) {
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
