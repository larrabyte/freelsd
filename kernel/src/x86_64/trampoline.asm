%include "kernel/src/x86_64/macros.asm"

global smpdata
extern gdt64.hiptr, gdt64.ptr
extern gdt64.code, gdt64.data

; Global definitions.
APMAGIC     equ 0xCA55E77E
SMPEXECUTE  equ 0x00001000
SMPSHARED   equ 0x00002000
FARJMPADDR  equ SMPEXECUTE + (aplongmode - smpstart)

section .data
smpdata:
dq smpstart                               ; Starting address of the SMP trampoline code.
dq smpsize                                ; Size of the SMP trampoline bytecode.
dq SMPEXECUTE                             ; The address where APs are to begin execution.
dq SMPSHARED                              ; The address to for sharing data between the BSP and the APs.
dd APMAGIC                                ; The expected magic value when the APs are initialising.

section .text
[BITS 16]
smpstart:
    cli                                   ; Disable external interrupts.
    cld                                   ; Zero the direction flag.
    mov eax, [SMPSHARED + 0x8]            ; Move smpcomm.cr3 (offset 0x8) into eax.
    mov cr3, eax                          ; Move this value into the AP's cr3 register.

    mov ecx, 0xC0000080                   ; Write the MSR that we want into ecx (the EFER).
    rdmsr                                 ; Read the MSR into eax.
    or eax, 1 << 8                        ; Enable the long mode bit.
    wrmsr                                 ; Write eax back into the EFER.

    mov eax, cr4                          ; Read the contents of cr4 into eax.
    or eax, 1 << 5                        ; Enable the PAE bit.
    mov cr4, eax                          ; Write eax back into cr4.

    mov eax, cr0                          ; Read the contents of cr0 into eax.
    or eax, 1 << 31                       ; Enable the paging bit.
    or eax, 1 << 0                        ; Enable the protected mode bit.
    mov cr0, eax                          ; Write back into cr0, enabling compatiability mode (64-bit).

    lgdt [SMPSHARED + 0x20]               ; Load the GDTR with the temporary pointer from the BSP.
    jmp gdt64.code:FARJMPADDR             ; Far jump to fully fledged long mode.

[BITS 64]
aplongmode:
    lgdt [gdt64.hiptr]                    ; Load the proper GDT64 pointer.
    mov ax, gdt64.data                    ; Move the data descriptor's index into ax.
    mov ss, ax                            ; Set ss to use the data descriptor.
    mov ds, ax                            ; Set ds to use the data descriptor.
    mov es, ax                            ; Set es to use the data descriptor.
    mov fs, ax                            ; Set fs to use the data descriptor.
    mov gs, ax                            ; Set gs to use the data descriptor.

    mov rax, cr0                          ; Copy contents of cr0 into rax.
    or rax, 1 << 1                        ; Enable bit 1 of cr0: Monitor Coprocessor (MP).
    and ax, 0xFFFB                        ; Clear bit 2 of cr0: Emulate Coprocessor (EM).
    mov cr0, rax                          ; Store the result back into cr0.

    mov rax, cr4                          ; Copy contents of cr4 into rax.
    or rax, 1 << 9                        ; Enable bit 9 of cr4: FXSAVE/FXRSTOR support (OXFXSR).
    or rax, 1 << 10                       ; Enable bit 10 of cr4: Unmasked Exception support (OSMMEXCPT).
    mov cr4, rax                          ; Store the result back into cr4.

    mov rsp, [SMPSHARED + 0x10]           ; Move smpcomm.rsp (offset 0x10) into rsp.
    mov rax, [SMPSHARED + 0x18]           ; Move smpcomm.entry (offset 0x18) into rax.
    call rax                              ; Call the entry function in rax.
    hlt                                   ; Halt the processor.

smpsize: equ $ - smpstart
