MAGIC    equ 0xE85250D6
ARCH     equ 0
LENGTH   equ hdrend - hdrstart
CHECKSUM equ -(MAGIC + ARCH + LENGTH)

global bootstrap
extern kernelmain
extern gdt64.pointer
extern gdt64.code
extern gdt64.data
extern _init
extern _fini

section .text
[BITS 32]
bootstrap:
    cli                      ; Disable interrupts.
    mov esp, stacktop        ; Set esp to our new stacktop.
    push eax                 ; Store the multiboot magic number.
    push ebx                 ; Store the multiboot struct.

    mov edi, 0x1000          ; Move the PML4's address into edi.
    mov cr3, edi             ; Move the PML4's address into cr3.
    mov eax, 0               ; Zero out the eax register.
    mov ecx, 1024            ; How many bytes to repeat for.
    rep stosd                ; memset(&pml4, 0, 4096);
    mov edi, cr3             ; Reset edi to the PML4's address.

    mov dword [edi], 0x2003  ; Point the first PML4 entry to the first PDPT.
    add edi, 0x1000          ; Set edi to the address of the first PDPT.
    mov dword [edi], 0x3003  ; Point the first PDPT to the first PDT.
    add edi, 0x1000          ; Set edi to the address of the first PDT.
    mov dword [edi], 0x4003  ; Point the first PDT to the first PT.
    add edi, 0x1000          ; Set edi to the address of the first PT.

    mov ebx, 0x3             ; Set ebx to a template page table entry.
    mov ecx, 512             ; How many page table entries to set.

.ptentry:
    mov dword [edi], ebx     ; Move our entries into the address in edi.
    add ebx, 0x1000          ; Add 0x1000 to our page table entry.
    add edi, 8               ; Move along eight bytes.
    loop .ptentry            ; Repeat until ecx is zero.

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

    pop ebx                  ; Restore the multiboot struct address.
    pop eax                  ; Restore the multiboot magic number.
    lgdt [gdt64.pointer]     ; Load the GDTR with our 64-bit GDT.
    jmp gdt64.code:longmode  ; Far jump to load our GDT and switch to long mode.

[BITS 64]
longmode:
    mov cx, gdt64.data       ; Move the data descriptor into cx.
    mov ss, cx               ; Set the stack segment to cx.
    mov ds, cx               ; Set the data segment to cx.
    mov es, cx               ; Set the extra segment to cx.
    mov fs, cx               ; Set the F segment to cx.
    mov gs, cx               ; Set the G segment to cx.

    mov r12, rax             ; Move the magic number into a preserved register.
    mov r13, rbx             ; Move the struct address into a preserved register.
    call _init               ; Initialise global constructors.

    mov rdi, r12             ; rdi is the first argument in the x86_64 System V ABI.
    mov rsi, r13             ; rsi is the second argument in the x86_64 System V ABI.
    call kernelmain          ; Start FreeLSD.

    call _fini               ; Kernel return? OK.
    hlt                      ; Halt the processor.

section .mbheader
align 8
hdrstart:
dd MAGIC
dd ARCH
dd LENGTH
dd CHECKSUM
dd 0
dd 8
hdrend:

section .bss
align 16
resb 4096
stacktop:
