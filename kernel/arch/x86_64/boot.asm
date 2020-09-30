%include "kernel/arch/x86_64/macros.asm"

global bootstrap, pge64sel
extern gdt64.ptr, gdt64.code, gdt64.data
extern _init, kernelmain
extern startbss, endbss

; Multiboot 2 header definitions.
; See section .mbheader for tags.
MAGIC        equ 0xE85250D6
ARCH         equ 0x00000000
LENGTH       equ hdrend - hdrstart
CHECKSUM     equ -(MAGIC + ARCH + LENGTH)

; Bootstrap paging information and data structures.
; Should allow for an initial 1GB of address space.
PML4_VOFFSET equ ((KERNEL_VBASE >> 39) & 0x1FF) * 8
PDPT_VOFFSET equ ((KERNEL_VBASE >> 30) & 0x1FF) * 8

PML4         equ 0x0000000000000000
PDPTPHYS     equ 0x0000000000001000
PDPTVIRT     equ 0x0000000000002000
PGESIZE      equ PDPTVIRT - PML4 + 0x1000

section .data
pge64sel:
dq PML4                         ; The start address of the bootstrap paging structures.
dq PDPTVIRT                     ; The end address of the bootstrap paging structures.
dq PGESIZE                      ; The size of the bootstrap paging structures.

section .bss
align 16                        ; 16-byte alignment to conform with compiler.
resb 16384                      ; Reserve 16KB in the kernel's BSS section for the stack.
mb2stack: equ $ - KERNEL_VBASE  ; Top of the stack, label for protected mode (32-bit range).
stacktop: equ $                 ; Top of the stack, label for long mode (64-bit range).

section .mbheader
hdrstart:
dd MAGIC                        ; Denotes the Multiboot2 magic value.
dd ARCH                         ; The specified architecture of our CPU.
dd LENGTH                       ; Length of the header, including tags.
dd CHECKSUM                     ; Checksum to verify header integrity.

align 8                         ; Multiboot2 tags must be 8-byte aligned.
fbtag.start:                    ; Start of the framebuffer tag.
dw MB2TAG_FRAMEBUFFER           ; Denotes the tag's type: framebuffer.
dw 0                            ; No flags required in an MB2TAG_FRAMEBUFFER.
dd fbtag.end - fbtag.start      ; Size of the tag in bytes.
dd 0                            ; Requested width.
dd 0                            ; Requested height.
dd 0                            ; Requested depth.
fbtag.end:                      ; End of the framebuffer tag.

align 8                         ; Multiboot2 tags must be 8-byte aligned.
dw MB2TAG_END                   ; Denotes the end of tags in the header.
dw 0                            ; No flags required in an MB2TAG_END.
dd 8                            ; Size of an end tag is always 8.
hdrend:                         ; End of the Multiboot2 header.

section .mbtext
[BITS 32]
bootstrap:
    cli                         ; Disable interrupts.
    mov edx, eax                ; Preserve contents of eax in edx.
    mov esp, mb2stack           ; Setup the stack pointer.

    push dword 0                ; Push a zero 64-bit integer.
    or dword [esp], 1 << 1      ; Keep reserved bit 1 set.
    or dword [esp], 1 << 21     ; Test bit 21 of EFLAGS: CPUID support.
    popfd                       ; Pop into EFLAGS.

    pushfd                      ; Repush EFLAGS.
    pop eax                     ; Pop it into eax.
    test eax, 1 << 21           ; Test if bit 21 is still set.
    mov eax, 0x1                ; If panicing, set eax to indicate lack of CPUID support.
    jz earlypanic               ; Panic if the test instruction returned zero.

    mov edi, startbss           ; Set edi to the kernel's BSS section.
    mov ecx, endbss             ; Set ecx to the end of the BSS section.
    sub ecx, edi                ; Calculate the size of the BSS and store in ecx.
    xor eax, eax                ; Zero out eax so we can do a memset().
    rep stosb                   ; Repeatedly zero memory in 1-byte chunks.
    push edx                    ; Now the stack is ready, push the multiboot magic.
    push ebx                    ; Now the stack is ready, push the multiboot struct address.

    mov eax, 0x80000000         ; Set the CPUID leaf to read (highest extended function).
    cpuid                       ; Execute CPUID to get highest available function.
    cmp eax, 0x80000001         ; Compare highest leaf with extended processor info.
    mov eax, 0x2                ; If panicing, set eax to indicate lack of extended info.
    jb earlypanic               ; Panic if highest function is less than 0x80000001.

    mov eax, 0x80000001         ; Set eax to get extended processor info from CPUID.
    cpuid                       ; Execute CPUID again. 

    test edx, 1 << 29           ; Test bit 29 of edx: Long Mode (LM).
    mov eax, 0x3                ; If panicing, set eax to indicate LM non-support.
    jz earlypanic               ; If the bit isn't set, then panic!

    test edx, 1 << 26           ; Test bit 26 of edx: Gigabyte Pages (PDPE1GB).
    mov eax, 0x4                ; If panicing, set eax to indicate PDPE1GB non-support.
    jz earlypanic               ; If the bit isn't set, then panic!

    mov edi, PML4               ; Move the base paging structure's address into edi.
    mov cr3, edi                ; Move the PML4's address into cr3.
    mov ecx, 3072               ; Set ecx to the size of the PML4 and both PDPTs.
    xor eax, eax                ; Zero out eax, this is what we want in memory.
    rep stosd                   ; Repeatedly zero memory in 4-byte chunks.

    mov edi, PML4               ; Move the base address of the PML4 into edi.
    mov eax, PDPTPHYS           ; Move the address of the lower PDPT into eax.
    or eax, 1 << 0              ; Enable the present flag for the PML4 entry.
    or eax, 1 << 1              ; Enable the read/write flag for the PML4 entry.
    mov dword [edi], eax        ; Point the first entry of the PML4 to the lower PDPT.

    add edi, PML4_VOFFSET       ; Add the virtual PDPT's offset in the PML4 to edi.
    mov eax, PDPTVIRT           ; Move the address of the higher PDPT into eax.
    or eax, 1 << 0              ; Enable the present flag for the PML4 entry.
    or eax, 1 << 1              ; Enable the read/write flag for the PML4 entry.
    mov dword [edi], eax        ; Point the higher entry of the PML4 to the higher PDPT.

    mov edi, PDPTPHYS           ; Set edi to the lower PDPT's address.
    mov dword [edi], 0x83       ; Set the first entry as a 1GB page, present and read/write.

    mov edi, PDPTVIRT           ; Set edi to the higher PDPT's address.
    add edi, PDPT_VOFFSET       ; Add the higher offset onto edi.
    mov dword [edi], 0x83       ; Set the entry as a 1GB page, present and read/write.

    mov eax, cr4                ; Copy contents of cr4 into eax.
    or eax, 1 << 5              ; Enable the PAE bit.
    mov cr4, eax                ; Copy contents of eax into cr4.

    mov ecx, 0xC0000080         ; Set the MSR to read (EFER).
    rdmsr                       ; Read the EFER register into eax.
    or eax, 1 << 8              ; Enable the long mode bit.
    wrmsr                       ; Write eax back into the EFER register.

    mov eax, cr0                ; Copy contents of cr0 into eax.
    or eax, 1 << 31             ; Enable the paging bit.
    or eax, 1 << 0              ; Enable the protected mode bit.
    mov cr0, eax                ; Write eax back into cr0.

    mov ax, gdt64.data          ; Move the data descriptor into ax.
    lgdt [gdt64.ptr]            ; Load the GDTR with a pointer to the GDT.
    jmp gdt64.code:longmode     ; Far jump to load our GDT and switch to long mode.

earlypanic:
    bcast32 0xDEADC0DE          ; Broadcast 0xDEADC0DE to all GPRs.
    ud2                         ; Force a triple fault.

[BITS 64]
longmode:
    mov ss, ax                  ; Set ss to use the data descriptor.
    mov ds, ax                  ; Set ds to use the data descriptor.
    mov es, ax                  ; Set es to use the data descriptor.
    mov fs, ax                  ; Set fs to use the data descriptor.
    mov gs, ax                  ; Set gs to use the data descriptor.

    mov rcx, gdt64.ptr          ; Set rcx to the address of the GDT pointer.
    mov rax, [rcx + 0x2]        ; Load the GDT pointer's value into rcx.
    add rax, KERNEL_VBASE       ; Add the kernel's virtual offset to the pointer.
    mov [rcx + 0x2], rax        ; Move the new pointer value back into the GDT's pointer.
    lgdt [rcx]                  ; Reload the GDTR with our new virtual address.

    mov rax, cr0                ; Copy contents of cr0 into rax.
    or rax, 1 << 1              ; Enable bit 1 of cr0: Monitor Coprocessor (MP).
    and ax, 0xFFFB              ; Clear bit 2 of cr0: Emulate Coprocessor (EM).
    mov cr0, rax                ; Store the result back into cr0.

    mov rax, cr4                ; Copy contents of cr4 into rax.
    or rax, 1 << 9              ; Enable bit 9 of cr4: FXSAVE/FXRSTOR support (OXFXSR).
    or rax, 1 << 10             ; Enable bit 10 of cr4: Unmasked Exception support (OSMMEXCPT).
    mov cr4, rax                ; Store the result back into cr4.

    pop rbx                     ; Pop both the multiboot magic (upper) and struct (lower) into rax.
    mov rsp, stacktop           ; Load the stack pointer with the kernel's proper stack address.
    mov rax, _init              ; Move the full 64-bit address of _init into rax.
    call rax                    ; Call rax and initialise global constructors.

    mov esi, ebx                ; Move the lower 32-bits of ebx into esi and zero extend to 64-bits.
    mov rdi, rbx                ; Move the whole of rbx into rdi.
    shr rdi, 32                 ; Shift rdi by 32-bits to retrieve magic value.
    mov rax, kernelmain         ; Move the full 64-bit address of kernelmain into rax.
    call rax                    ; Call the address and start FreeLSD.
