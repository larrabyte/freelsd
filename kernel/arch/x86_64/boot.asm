%include "kernel/arch/x86_64/macros.asm"

global bootstrap
global pge64s
global pge64e
global pge64l

extern gdt64.pointer64
extern gdt64.pointer
extern gdt64.code
extern gdt64.data

extern startcrtbss
extern endcrtbss
extern startbss
extern endbss

extern kernelmain
extern _init
extern _fini

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

section .mbtext
[BITS 32]
bootstrap:
    cli                      ; Disable interrupts.
    mov esi, eax             ; Store the multiboot magic into esi.
    xor eax, eax             ; Zero out the eax register.

    mov edi, startcrtbss     ; Set edi to crtbegin.o's BSS section.
    mov ecx, endcrtbss       ; Set ecx to the end of the BSS section.
    sub ecx, edi             ; Calculate the size of the BSS and store in ecx.
    rep stosb                ; Zero out memory in 1-byte chunks.

    mov edi, startbss        ; Set edi to the kernel's BSS section.
    mov ecx, endbss          ; Set ecx to the end of the BSS section.
    sub ecx, edi             ; Calculate the size of the BSS and store in ecx.
    rep stosb                ; Repeatedly zero memory in 1-byte chunks.

    mov edi, PML4            ; Move the base paging structure's address into edi.
    mov cr3, edi             ; Move the PML4's address into cr3.
    mov ecx, 3072            ; Set ecx to the size of the PML4 and both PDPTs.
    rep stosd                ; Repeatedly zero memory in 4-byte chunks.

    mov edi, PML4            ; Move the base address of the PML4 into edi.
    mov eax, PDPTPHYS        ; Move the address of the lower PDPT into eax.
    or eax, 1 << 0           ; Enable the present flag for the PML4 entry.
    or eax, 1 << 1           ; Enable the read/write flag for the PML4 entry.
    mov dword [edi], eax     ; Point the first entry of the PML4 to the lower PDPT.

    add edi, PML4_VOFFSET    ; Add the virtual PDPT's offset in the PML4 to edi.
    mov eax, PDPTVIRT        ; Move the address of the higher PDPT into eax.
    or eax, 1 << 0           ; Enable the present flag for the PML4 entry.
    or eax, 1 << 1           ; Enable the read/write flag for the PML4 entry.
    mov dword [edi], eax     ; Point the higher entry of the PML4 to the higher PDPT.

    mov edi, PDPTPHYS        ; Set edi to the lower PDPT's address.
    mov dword [edi], 0x83    ; Set the first entry as a 1GB page, present and read/write.

    mov edi, PDPTVIRT        ; Set edi to the higher PDPT's address.
    add edi, PDPT_VOFFSET    ; Add the higher offset onto edi.
    mov dword [edi], 0x83    ; Set the entry as a 1GB page, present and read/write.

    mov eax, cr4             ; Copy contents of cr4 into eax.
    or eax, 1 << 5           ; Enable the PAE bit.
    mov cr4, eax             ; Copy contents of eax into cr4.

    mov ecx, 0xC0000080      ; Set the MSR to read (EFER).
    rdmsr                    ; Read the EFER register into eax.
    or eax, 1 << 8           ; Enable the long mode bit.
    wrmsr                    ; Write eax back into the EFER register.

    mov eax, cr0             ; Copy contents of cr0 into eax.
    or eax, 1 << 31          ; Enable the paging bit.
    or eax, 1 << 0           ; Enable the protected mode bit.
    mov cr0, eax             ; Write eax back into cr0.
    lgdt [gdt64.pointer]     ; Load the GDTR with the "real" address of the GDT.

    ; Far jump to load our GDT and switch to long mode.
    jmp gdt64.code:longmode - KERNEL_VBASE

section .text
[BITS 64]
longmode:
    lgdt [gdt64.pointer64]   ; Reload the GDTR with the virtual address of the GDT.
    mov cx, gdt64.data       ; Move the data descriptor into cx.
    mov ss, cx               ; Set the stack segment to cx.
    mov ds, cx               ; Set the data segment to cx.
    mov es, cx               ; Set the extra segment to cx.
    mov fs, cx               ; Set the F segment to cx.
    mov gs, cx               ; Set the G segment to cx.

    mov rsp, stacktop        ; Setup the stack pointer now that paging is done.
    push 0                   ; Push a zero 64-bit integer.
    popf                     ; Zero out RFLAGS.

    push rsi                 ; Push the multiboot magic number to the stack.
    push rbx                 ; Push the multiboot struct address onto the stack.
    call _init               ; Initialise global constructors.

    pop rsi                  ; rsi is the second argument in the x86_64 System V ABI.
    pop rdi                  ; rdi is the first argument in the x86_64 System V ABI.
    jmp kernelmain           ; Start FreeLSD.

section .mbheader
align 8
hdrstart:
dd MAGIC
dd ARCH
dd LENGTH
dd CHECKSUM

align 8
fbtag.start:
dw MULTIBOOT2_HEADERTAG_FRAMEBUFFER
dw 0
dd fbtag.end - fbtag.start
dd 0
dd 0
dd 0
fbtag.end:

align 8
dw MULTIBOOT2_HEADERTAG_END
dw 0
dd 8
hdrend:

section .data
pge64s:
dq PML4
pge64e:
dq PDPTVIRT
pge64l:
dq PGESIZE

section .bss
align 16
resb 16384
stacktop:
