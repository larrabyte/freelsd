OUTPUT_FORMAT(elf64-x86-64)
ENTRY(main)

PHDRS {
    /* Set Limine's page tables to have the appropriate
    permissions set for when the kernel begins executing.

    Bit 0 marks the section as executable.
    Bit 1 marks the section as writable.
    Bit 2 marks the section as readable. */

    text PT_LOAD FLAGS((1 << 0));
    rodata PT_LOAD FLAGS((1 << 2));
    data PT_LOAD FLAGS((1 << 1) | (1 << 2));
    bss PT_LOAD FLAGS((1 << 1) | (1 << 2));
}

SECTIONS {
    . = 0xFFFFFFFF80000000;

    .text ALIGN(CONSTANT(MAXPAGESIZE)) : {
        *(.text*)
    } : text

    . += CONSTANT(MAXPAGESIZE);

    .rodata ALIGN(CONSTANT(MAXPAGESIZE)) : {
        *(.rodata*)
    } : rodata

    . += CONSTANT(MAXPAGESIZE);

    .data ALIGN(CONSTANT(MAXPAGESIZE)) : {
        *(.data*)
    } : data

    . += CONSTANT(MAXPAGESIZE);

    .bss ALIGN(CONSTANT(MAXPAGESIZE)) :  {
        *(.bss*)
    } : bss
}
