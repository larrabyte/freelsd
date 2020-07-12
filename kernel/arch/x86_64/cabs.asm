global loadcr3
global readcpuid

loadcr3:
    mov cr3, rdi                ; Move the address in rdi to cr3.
    ret                         ; Return.

readcpuid:
    mov eax, edi                ; Move the CPUID leaf specifier into eax.
    cpuid                       ; Execute the CPUID instruction.
    mov dword [rsi + 0x0], eax  ; Move eax into the cpuid_t struct (offset 0x0).
    mov dword [rsi + 0x4], ebx  ; Move ebx into the cpuid_t struct (offset 0x4).
    mov dword [rsi + 0x8], ecx  ; Move ecx into the cpuid_t struct (offset 0x8).
    mov dword [rsi + 0xC], edx  ; Move edx into the cpuid_t struct (offset 0xC).
    ret                         ; Return.
