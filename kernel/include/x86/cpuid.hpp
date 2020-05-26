#ifndef FREELSD_KERNEL_CPUID_HEADER
#define FREELSD_KERNEL_CPUID_HEADER

#include <stdint.h>

// --------------------------------------------------
// Check for supported CPU features using these bits.
// --------------------------------------------------

#define CPUID_FEATURE_ECX_SSE3     0x00000001
#define CPUID_FEATURE_ECX_PCLMUL   0x00000002
#define CPUID_FEATURE_ECX_DTES64   0x00000004
#define CPUID_FEATURE_ECX_MONITOR  0x00000008
#define CPUID_FEATURE_ECX_DS_CPL   0x00000010
#define CPUID_FEATURE_ECX_VMX      0x00000020
#define CPUID_FEATURE_ECX_SMX      0x00000040
#define CPUID_FEATURE_ECX_EST      0x00000080
#define CPUID_FEATURE_ECX_TM2      0x00000100
#define CPUID_FEATURE_ECX_SSSE3    0x00000200
#define CPUID_FEATURE_ECX_CID      0x00000400
#define CPUID_FEATURE_ECX_FMA      0x00001000
#define CPUID_FEATURE_ECX_CX16     0x00002000
#define CPUID_FEATURE_ECX_ETPRD    0x00004000
#define CPUID_FEATURE_ECX_PDCM     0x00008000
#define CPUID_FEATURE_ECX_PCIDE    0x00020000
#define CPUID_FEATURE_ECX_DCA      0x00040000
#define CPUID_FEATURE_ECX_SSE41    0x00080000
#define CPUID_FEATURE_ECX_SSE42    0x00100000
#define CPUID_FEATURE_ECX_X2APIC   0x00200000
#define CPUID_FEATURE_ECX_MOVBE    0x00400000
#define CPUID_FEATURE_ECX_POPCNT   0x00800000
#define CPUID_FEATURE_ECX_AES      0x02000000
#define CPUID_FEATURE_ECX_XSAVE    0x04000000
#define CPUID_FEATURE_ECX_OSXSAVE  0x08000000
#define CPUID_FEATURE_ECX_AVX      0x10000000

#define CPUID_FEATURE_EDX_FPU      0x00000001
#define CPUID_FEATURE_EDX_VME      0x00000002
#define CPUID_FEATURE_EDX_DE       0x00000004
#define CPUID_FEATURE_EDX_PSE      0x00000008
#define CPUID_FEATURE_EDX_TSC      0x00000010
#define CPUID_FEATURE_EDX_MSR      0x00000020
#define CPUID_FEATURE_EDX_PAE      0x00000040
#define CPUID_FEATURE_EDX_MCE      0x00000080
#define CPUID_FEATURE_EDX_CX8      0x00000100
#define CPUID_FEATURE_EDX_APIC     0x00000200
#define CPUID_FEATURE_EDX_SEP      0x00000800
#define CPUID_FEATURE_EDX_MTRR     0x00001000
#define CPUID_FEATURE_EDX_PGE      0x00002000
#define CPUID_FEATURE_EDX_MCA      0x00004000
#define CPUID_FEATURE_EDX_CMOV     0x00008000
#define CPUID_FEATURE_EDX_PAT      0x00010000
#define CPUID_FEATURE_EDX_PSE36    0x00020000
#define CPUID_FEATURE_EDX_PSN      0x00040000
#define CPUID_FEATURE_EDX_CLF      0x00080000
#define CPUID_FEATURE_EDX_DTES     0x00200000
#define CPUID_FEATURE_EDX_ACPI     0x00400000
#define CPUID_FEATURE_EDX_MMX      0x00800000
#define CPUID_FEATURE_EDX_FXSR     0x01000000
#define CPUID_FEATURE_EDX_SSE      0x02000000
#define CPUID_FEATURE_EDX_SSE2     0x04000000
#define CPUID_FEATURE_EDX_SS       0x08000000
#define CPUID_FEATURE_EDX_HTT      0x10000000
#define CPUID_FEATURE_EDX_TM1      0x20000000
#define CPUID_FEATURE_EDX_IA64     0x40000000
#define CPUID_FEATURE_EDX_PBE      0x80000000

// --------------------------------------------
// CPUID vendor strings from CPU manufacturers.
// --------------------------------------------

#define CPUID_VENDOR_OLDAMD        "AMDisbetter!"
#define CPUID_VENDOR_AMD           "AuthenticAMD"
#define CPUID_VENDOR_INTEL         "GenuineIntel"
#define CPUID_VENDOR_OLDTRANSMETA  "TransmetaCPU"
#define CPUID_VENDOR_TRANSMETA     "GenuineTMx86"
#define CPUID_VENDOR_CYRIX         "CyrixInstead"
#define CPUID_VENDOR_CENTAUR       "CentaurHauls"
#define CPUID_VENDOR_NEXGEN        "NexGenDriven"
#define CPUID_VENDOR_UMC           "UMC UMC UMC "
#define CPUID_VENDOR_SIS           "SiS SiS SiS "
#define CPUID_VENDOR_NSC           "Geode by NSC"
#define CPUID_VENDOR_RISE          "RiseRiseRise"
#define CPUID_VENDOR_VORTEX        "Vortex86 SoC"
#define CPUID_VENDOR_VIA           "VIA VIA VIA "

// -------------------------------------------
// CPUID vendor strings from virtual machines.
// -------------------------------------------

#define CPUID_VENDOR_VMWARE        "VMwareVMware"
#define CPUID_VENDOR_XENHVM        "XenVMMXenVMM"
#define CPUID_VENDOR_MICROSOFT_HV  "Microsoft Hv"
#define CPUID_VENDOR_PARALLELS     " lrpepyh vr"

typedef struct cpuid_info {
    char vendor[13];
    uint32_t ecx;
    uint32_t edx;
} cpuid_info_t;

namespace cpu {
    // Retrieves processor information via CPUID.
    cpuid_info_t executecpuid(void);
}

#endif
