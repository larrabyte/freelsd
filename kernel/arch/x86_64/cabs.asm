global loadcr3
global idtflush
global readcpuid

section .text
loadcr3:
    mov cr3, rdi                ; Move a new PML4 base address into CR3.
    ret                         ; Return.

idtflush:
    lidt [rdi]                  ; Load the LDTR with the address specified in rdi.
    sti                         ; Re-enable interrupts.
    ret                         ; Return.

readcpuid:
    mov eax, edi                ; Move the CPUID leaf specifier into eax.
    push rbx                    ; Save rbx as it is a preserved register.
    cpuid                       ; Execute the CPUID instruction.

    mov dword [rsi + 0x0], eax  ; Move eax into the cpuid_t struct (offset 0x0).
    mov dword [rsi + 0x4], ebx  ; Move ebx into the cpuid_t struct (offset 0x4).
    mov dword [rsi + 0x8], ecx  ; Move ecx into the cpuid_t struct (offset 0x8).
    mov dword [rsi + 0xC], edx  ; Move edx into the cpuid_t struct (offset 0xC).
    pop rbx                     ; Restore rbx.
    ret                         ; Return.
