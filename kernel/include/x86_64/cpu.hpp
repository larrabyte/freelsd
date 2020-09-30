#ifndef FREELSD_KERNEL_CPU_HEADER
#define FREELSD_KERNEL_CPU_HEADER

#include <interrupts.hpp>
#include <stdint.h>

#define CPUID_EDX_BIT 0x8000000000000000

typedef enum cpuidfeature {
    // Features specified in the ecx register.
    CPU_FEATURE_SSE3    = (1UL << 0),
    CPU_FEATURE_PCLMUL  = (1UL << 1),
    CPU_FEATURE_DTES64  = (1UL << 2),
    CPU_FEATURE_MONITOR = (1UL << 3),
    CPU_FEATURE_DSCPL   = (1UL << 4),
    CPU_FEATURE_VMX     = (1UL << 5),
    CPU_FEATURE_SMX     = (1UL << 6),
    CPU_FEATURE_EST     = (1UL << 7),
    CPU_FEATURE_TM2     = (1UL << 8),
    CPU_FEATURE_SSSE3   = (1UL << 9),
    CPU_FEATURE_L1CID   = (1UL << 10),
    CPU_FEATURE_SDBG    = (1UL << 11),
    CPU_FEATURE_FMA     = (1UL << 12),
    CPU_FEATURE_CX16    = (1UL << 13),
    CPU_FEATURE_XPTR    = (1UL << 14),
    CPU_FEATURE_PDCM    = (1UL << 15),
    CPU_FEATURE_PCID    = (1UL << 17),
    CPU_FEATURE_DCA     = (1UL << 18),
    CPU_FEATURE_SSE41   = (1UL << 19),
    CPU_FEATURE_SSE42   = (1UL << 20),
    CPU_FEATURE_X2APIC  = (1UL << 21),
    CPU_FEATURE_MOVBE   = (1UL << 22),
    CPU_FEATURE_POPCNT  = (1UL << 23),
    CPU_FEATURE_TSCDLN  = (1UL << 24),
    CPU_FEATURE_AES     = (1UL << 25),
    CPU_FEATURE_XSAVE   = (1UL << 26),
    CPU_FEATURE_OSXSAVE = (1UL << 27),
    CPU_FEATURE_AVX     = (1UL << 28),
    CPU_FEATURE_F16C    = (1UL << 29),
    CPU_FEATURE_RDRND   = (1UL << 30),
    CPU_FEATURE_HVISOR  = (1UL << 31),

    // Features specified in the edx register.
    CPU_FEATURE_FPU     = (1UL << 63) | (1 << 0),
    CPU_FEATURE_VME     = (1UL << 63) | (1 << 1),
    CPU_FEATURE_DEBUG   = (1UL << 63) | (1 << 2),
    CPU_FEATURE_PSE     = (1UL << 63) | (1 << 3),
    CPU_FEATURE_TSC     = (1UL << 63) | (1 << 4),
    CPU_FEATURE_MSR     = (1UL << 63) | (1 << 5),
    CPU_FEATURE_PAE     = (1UL << 63) | (1 << 6),
    CPU_FEATURE_MCE     = (1UL << 63) | (1 << 7),
    CPU_FEATURE_CX8     = (1UL << 63) | (1 << 8),
    CPU_FEATURE_APIC    = (1UL << 63) | (1 << 9),
    CPU_FEATURE_SEP     = (1UL << 63) | (1 << 11),
    CPU_FEATURE_MTRR    = (1UL << 63) | (1 << 12),
    CPU_FEATURE_PGE     = (1UL << 63) | (1 << 13),
    CPU_FEATURE_MCA     = (1UL << 63) | (1 << 14),
    CPU_FEATURE_CMOV    = (1UL << 63) | (1 << 15),
    CPU_FEATURE_PAT     = (1UL << 63) | (1 << 16),
    CPU_FEATURE_PSE36   = (1UL << 63) | (1 << 17),
    CPU_FEATURE_PSN     = (1UL << 63) | (1 << 18),
    CPU_FEATURE_CLFSH   = (1UL << 63) | (1 << 19),
    CPU_FEATURE_DS      = (1UL << 63) | (1 << 21),
    CPU_FEATURE_ACPI    = (1UL << 63) | (1 << 22),
    CPU_FEATURE_MMX     = (1UL << 63) | (1 << 23),
    CPU_FEATURE_FXSR    = (1UL << 63) | (1 << 24),
    CPU_FEATURE_SSE     = (1UL << 63) | (1 << 25),
    CPU_FEATURE_SSE2    = (1UL << 63) | (1 << 26),
    CPU_FEATURE_SS      = (1UL << 63) | (1 << 27),
    CPU_FEATURE_HTT     = (1UL << 63) | (1 << 28),
    CPU_FEATURE_TM      = (1UL << 63) | (1 << 29),
    CPU_FEATURE_IA64    = (1UL << 63) | (1 << 30),
    CPU_FEATURE_PBE     = (1UL << 63) | (1 << 31)
} cpuid_feature_t;

namespace cpu {
    typedef struct cpuid_registers {
        uint32_t eax;
        uint32_t ebx;
        uint32_t ecx;
        uint32_t edx;
    } cpuid_regs_t;

    typedef struct cpuid_stats {
        uint32_t stdmax, extmax;
        char vendor[13];
        char brand[48];
    } cpuid_stats_t;

    // Generic CPU exception handler.
    void handler(idt::regs64_t *regs);

    // Get the CPU vendor as a string.
    char *getvendor(void);

    // Get the CPU brand name.
    char *getbrandname(void);

    // Checks whether the CPU supports a certain feature.
    bool supports(cpuid_feature_t feature);

    // Get CPU information using CPUID.
    void initialisestats(void);
}

#endif
