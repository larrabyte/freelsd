#ifndef FREELSD_KERNEL_ISR_HEADER
#define FREELSD_KERNEL_ISR_HEADER

#include <stdint.h>

namespace isr {
    typedef struct registers {
        uint32_t ds;
        uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
        uint32_t intnum, errcode;
        uint32_t eip, cs, eflags, useresp, ss;
    } registers_t;
}

#endif