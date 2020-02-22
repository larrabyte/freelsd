#pragma once

#include <stddef.h>

namespace cstr {
    // Return the size of a C-style string minus the terminating NULL character.
    size_t len(const char *str);
}