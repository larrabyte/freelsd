#ifndef FREELSD_KERNEL_CSTR_HEADER
#define FREELSD_KERNEL_CSTR_HEADER

#include <stddef.h>
#include <stdint.h>

namespace cstr {
    // Copies the ASCII representation of num into buffer. Integers passed in with bases other than 10 will be treated as unsigned.
    char *itoa(int num, int base);

    // Reverses the string passed in, given an amount of letters to reverse.
    void reverse(char *str, size_t length);

    // Swaps the character values at memory locations a & b.
    void swap(char *a, char *b);

    // Return the size of a C-style string minus the terminating NULL character.
    size_t len(const char *str);
}

#endif