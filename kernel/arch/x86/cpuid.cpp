#include <mem/libc.hpp>
#include <cpuid.hpp>

namespace cpu {
    cpuid_info_t executecpuid(void) {
        uint32_t ebx, ecx, edx;
        cpuid_info_t info;

        // Get the vendor string via CPUID and copy it into cpuid_info_t.
        asm volatile("cpuid" : "=b" (ebx), "=c" (ecx), "=d" (edx) : "a" (0));
        memcpy(info.vendor + 4, &edx, 4);
        memcpy(info.vendor + 8, &ecx, 4);
        memcpy(info.vendor, &ebx, 4);
        info.vendor[12] = '\0';

        // Get CPU features via edx and ecx and copy them into cpuid_info_t.
        asm volatile("cpuid" : "=d" (edx), "=c" (ecx) : "a" (1));
        info.edx = edx; info.ecx = ecx;

        return info;  // Return cpuid_info_t.
    }
}
