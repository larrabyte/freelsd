#include <mem/libc.hpp>
#include <errors.hpp>
#include <cpu.hpp>
#include <cpuid.h>

namespace cpu {
    static const char *exceptionstr[] = {
        "Divide-by-zero exception.",
        "Debug exception.",
        "Non-maskable interrupt.",
        "Breakpoint exception.",
        "Overflow exception.",
        "Bound range exceeded.",
        "Invalid opcode.",
        "Device not available.",
        "Double fault exception.",
        "Reserved exception.",
        "Invalid task state segment.",
        "Segment not present.",
        "Stack segment fault.",
        "General protection fault.",
        "Page fault.",
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
    static char vendor[13];

    void handler(idt::regs64_t *regs) {
        // Call kernel panic w/ interrupt context.
        ctxpanic(regs, exceptionstr[regs->isr]);
    }

    char *getvendor(void) {
        // Execute CPUID with a leaf of 0.
        readcpuid(0, &registers);

        // Copy the vendor string (MSB -> LSB).
        memcpy(vendor + 0, &registers.ebx, 4);
        memcpy(vendor + 4, &registers.edx, 4);
        memcpy(vendor + 8, &registers.ecx, 4);

        // Zero out the string and return.
        vendor[12] = '\0';
        return vendor;
    }

    bool supports(cpuid_feature_t feature) {
        // Execute CPUID with a leaf of 1.
        readcpuid(1, &registers);

        switch(feature) { // Check the features in ecx and edx.
            case CPU_FEATURE_SSE3: return (registers.ecx & CPU_FEATURE_SSE3) ? true : false;
            case CPU_FEATURE_PCLMUL: return (registers.ecx & CPU_FEATURE_PCLMUL) ? true : false;
            case CPU_FEATURE_DTES64: return (registers.ecx & CPU_FEATURE_DTES64) ? true : false;
            case CPU_FEATURE_MONITOR: return (registers.ecx & CPU_FEATURE_MONITOR) ? true : false;
            case CPU_FEATURE_DSCPL: return (registers.ecx & CPU_FEATURE_MONITOR) ? true : false;
            case CPU_FEATURE_VMX: return (registers.ecx & CPU_FEATURE_VMX) ? true : false;
            case CPU_FEATURE_SMX: return (registers.ecx & CPU_FEATURE_SMX) ? true : false;
            case CPU_FEATURE_EST: return (registers.ecx & CPU_FEATURE_EST) ? true : false;
            case CPU_FEATURE_TM2: return (registers.ecx & CPU_FEATURE_TM2) ? true : false;
            case CPU_FEATURE_SSSE3: return (registers.ecx & CPU_FEATURE_SSSE3) ? true : false;
            case CPU_FEATURE_L1CID: return (registers.ecx & CPU_FEATURE_L1CID) ? true : false;
            case CPU_FEATURE_SDBG: return (registers.ecx & CPU_FEATURE_SDBG) ? true : false;
            case CPU_FEATURE_FMA: return (registers.ecx & CPU_FEATURE_FMA) ? true : false;
            case CPU_FEATURE_CX16: return (registers.ecx & CPU_FEATURE_CX16) ? true : false;
            case CPU_FEATURE_XPTR: return (registers.ecx & CPU_FEATURE_XPTR) ? true : false;
            case CPU_FEATURE_PDCM: return (registers.ecx & CPU_FEATURE_PDCM) ? true : false;
            case CPU_FEATURE_PCID: return (registers.ecx & CPU_FEATURE_PCID) ? true : false;
            case CPU_FEATURE_DCA: return (registers.ecx & CPU_FEATURE_DCA) ? true : false;
            case CPU_FEATURE_SSE41: return (registers.ecx & CPU_FEATURE_SSE41) ? true : false;
            case CPU_FEATURE_SSE42: return (registers.ecx & CPU_FEATURE_SSE42) ? true : false;
            case CPU_FEATURE_X2APIC: return (registers.ecx & CPU_FEATURE_X2APIC) ? true : false;
            case CPU_FEATURE_MOVBE: return (registers.ecx & CPU_FEATURE_MOVBE) ? true : false;
            case CPU_FEATURE_POPCNT: return (registers.ecx & CPU_FEATURE_POPCNT) ? true : false;
            case CPU_FEATURE_TSCDLN: return (registers.ecx & CPU_FEATURE_TSCDLN) ? true : false;
            case CPU_FEATURE_AES: return (registers.ecx & CPU_FEATURE_AES) ? true : false;
            case CPU_FEATURE_XSAVE: return (registers.ecx & CPU_FEATURE_XSAVE) ? true : false;
            case CPU_FEATURE_OSXSAVE: return (registers.ecx & CPU_FEATURE_OSXSAVE) ? true : false;
            case CPU_FEATURE_AVX: return (registers.ecx & CPU_FEATURE_AVX) ? true : false;
            case CPU_FEATURE_F16C: return (registers.ecx & CPU_FEATURE_F16C) ? true : false;
            case CPU_FEATURE_RDRND: return (registers.ecx & CPU_FEATURE_RDRND) ? true : false;
            case CPU_FEATURE_HVISOR: return (registers.ecx & CPU_FEATURE_HVISOR) ? true : false;
            case CPU_FEATURE_FPU: return (registers.edx & (CPU_FEATURE_FPU ^ 0x1000000000000000)) ? true : false;
            case CPU_FEATURE_VME: return (registers.edx & (CPU_FEATURE_VME ^ 0x1000000000000000)) ? true : false;
            case CPU_FEATURE_DEBUG: return (registers.edx & (CPU_FEATURE_DEBUG ^ 0x1000000000000000)) ? true : false;
            case CPU_FEATURE_PSE: return (registers.edx & (CPU_FEATURE_PSE ^ 0x1000000000000000)) ? true : false;
            case CPU_FEATURE_TSC: return (registers.edx & (CPU_FEATURE_TSC ^ 0x1000000000000000)) ? true : false;
            case CPU_FEATURE_MSR: return (registers.edx & (CPU_FEATURE_MSR ^ 0x1000000000000000)) ? true : false;
            case CPU_FEATURE_PAE: return (registers.edx & (CPU_FEATURE_PAE ^ 0x1000000000000000)) ? true : false;
            case CPU_FEATURE_MCE: return (registers.edx & (CPU_FEATURE_MCE ^ 0x1000000000000000)) ? true : false;
            case CPU_FEATURE_CX8: return (registers.edx & (CPU_FEATURE_CX8 ^ 0x1000000000000000)) ? true : false;
            case CPU_FEATURE_APIC: return (registers.edx & (CPU_FEATURE_APIC ^ 0x1000000000000000)) ? true : false;
            case CPU_FEATURE_SEP: return (registers.edx & (CPU_FEATURE_SEP ^ 0x1000000000000000)) ? true : false;
            case CPU_FEATURE_MTRR: return (registers.edx & (CPU_FEATURE_MTRR ^ 0x1000000000000000)) ? true : false;
            case CPU_FEATURE_PGE: return (registers.edx & (CPU_FEATURE_PGE ^ 0x1000000000000000)) ? true : false;
            case CPU_FEATURE_MCA: return (registers.edx & (CPU_FEATURE_MCA ^ 0x1000000000000000)) ? true : false;
            case CPU_FEATURE_CMOV: return (registers.edx & (CPU_FEATURE_CMOV ^ 0x1000000000000000)) ? true : false;
            case CPU_FEATURE_PAT: return (registers.edx & (CPU_FEATURE_PAT ^ 0x1000000000000000)) ? true : false;
            case CPU_FEATURE_PSE36: return (registers.edx & (CPU_FEATURE_PSE36 ^ 0x1000000000000000)) ? true : false;
            case CPU_FEATURE_PSN: return (registers.edx & (CPU_FEATURE_PSN ^ 0x1000000000000000)) ? true : false;
            case CPU_FEATURE_CLFSH: return (registers.edx & (CPU_FEATURE_CLFSH ^ 0x1000000000000000)) ? true : false;
            case CPU_FEATURE_DS: return (registers.edx & (CPU_FEATURE_DS ^ 0x1000000000000000)) ? true : false;
            case CPU_FEATURE_ACPI: return (registers.edx & (CPU_FEATURE_ACPI ^ 0x1000000000000000)) ? true : false;
            case CPU_FEATURE_MMX: return (registers.edx & (CPU_FEATURE_MMX ^ 0x1000000000000000)) ? true : false;
            case CPU_FEATURE_FXSR: return (registers.edx & (CPU_FEATURE_FXSR ^ 0x1000000000000000)) ? true : false;
            case CPU_FEATURE_SSE: return (registers.edx & (CPU_FEATURE_SSE ^ 0x1000000000000000)) ? true : false;
            case CPU_FEATURE_SSE2: return (registers.edx & (CPU_FEATURE_SSE2 ^ 0x1000000000000000)) ? true : false;
            case CPU_FEATURE_SS: return (registers.edx & (CPU_FEATURE_SS ^ 0x1000000000000000)) ? true : false;
            case CPU_FEATURE_HTT: return (registers.edx & (CPU_FEATURE_HTT ^ 0x1000000000000000)) ? true : false;
            case CPU_FEATURE_TM: return (registers.edx & (CPU_FEATURE_TM ^ 0x1000000000000000)) ? true : false;
            case CPU_FEATURE_IA64: return (registers.edx & (CPU_FEATURE_IA64 ^ 0x1000000000000000)) ? true : false;
            case CPU_FEATURE_PBE: return (registers.edx & (CPU_FEATURE_PBE ^ 0x1000000000000000)) ? true : false;
        }

        // Unknown feature?
        return false;
    }
}
