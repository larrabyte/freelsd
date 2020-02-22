#pragma once

#include <stddef.h>

namespace memory {
    // Copies n bytes from source -> dest.
	void *copy(const void *dest, const void *source, size_t n);
}