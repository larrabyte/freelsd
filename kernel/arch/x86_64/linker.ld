OUTPUT_FORMAT(elf64-x86-64)
ENTRY(bootstrap)

SECTIONS {
    /* Begin putting sections at 1 MB physical. */
    . = 1M;

    .mbheader : ALIGN(4K) {
        *(.mbheader)
    }

    /* Code section and read-only data. */
    .text : ALIGN(4K) {
        *(.text)
        *(.rodata*)
    }

    /* Read-write and read-only data. */
    .data : ALIGN(4K) {
        *(.data)
    }

    /* Read-write data (uninitialized) and the stack. */
    .bss : ALIGN(4K) {
        *(COMMON)
        *(.bss)
    }

    kernelend = ALIGN(4K);
}
