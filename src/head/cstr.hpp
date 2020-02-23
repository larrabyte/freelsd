#pragma once

#include <stddef.h>
#include <stdint.h>

namespace cstr {
    // Reverses the string passed in, given an amount of letters to reverse.
    void reverse(char *str, uint64_t length);

    // Swaps the character values at memory locations a & b.
    void swap(char *a, char *b);

    // Copies the ASCII representation of num into buffer. Will cause a buffer overflow if memory is not properly allocated. Integers passed in with bases other than 10 will be treated as unsigned.
    char *itoa(int64_t num, char *buffer, uint64_t base);

    // Return the size of a C-style string minus the terminating NULL character.
    size_t len(const char *str);
}