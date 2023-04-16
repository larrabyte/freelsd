OUTPUT_FORMAT(elf64-x86-64)
ENTRY(main)

PHDRS {
    /* This is so Limine's paging tables have the
    right permissions when it passes control to us.
    From bits 0-2: executable, writable and readable. */
    text PT_LOAD FLAGS((1 << 0) | (1 << 2));
    rodata PT_LOAD FLAGS((1 << 2));
    data PT_LOAD FLAGS((1 << 1) | (1 << 2));
    bss PT_LOAD FLAGS((1 << 1) | (1 << 2));
}

SECTIONS {
    . = 0xffffffff80000000;

    .text ALIGN(CONSTANT(MAXPAGESIZE)) : {
        *(.text*)
    }

    .rodata ALIGN(CONSTANT(MAXPAGESIZE)) : {
        *(.rodata*)
    }

    .data ALIGN(CONSTANT(MAXPAGESIZE)) : {
        *(.data*)
    }

    .bss ALIGN(CONSTANT(MAXPAGESIZE)) :  {
        *(.bss*)
    }
}
