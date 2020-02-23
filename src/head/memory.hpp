#pragma once

#include <stddef.h>

namespace memory {
    // Moves n bytes from source -> dest. Aware of overlapping memory regions.
    void *move(const void *dest, const void *soucre, size_t n);

    // Copies n bytes from source -> dest. Unaware of overlapping memory regions.
	void *copy(const void *dest, const void *source, size_t n);
}