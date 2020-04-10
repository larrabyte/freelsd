#ifndef FREELSD_KERNEL_MEMORY_PHYSALLOC_HEADER
#define FREELSD_KERNEL_MEMORY_PHYSALLOC_HEADER

#include <multiboot.hpp>
#include <stdint.h>
#include <stddef.h>

#define PMMGR_BITMAP_ARRAY_SIZE  32768  // The size of the bitmap array (in dwords).
#define PMMGR_BLOCK_SIZE         4096   // The size of a single block (in bytes).
#define PMMGR_BLOCKS_PER_BYTE    8      // The number of blocks managed by one byte.

extern "C" void *kernelend;

namespace physmem {
    extern size_t usedblocks;     // Number of used blocks.
    extern size_t maxblocks;      // Total number of memory blocks available.
    extern size_t totalsize;      // Total memory available (in kilobytes).

    // Find the first instance of n contiguous blocks of memory.
    int findfirstfree(size_t n);

    // Allocate n physical blocks.
    void *allocblocks(size_t n);

    // Free n physical blocks, starting at base.
    void freeblocks(uintptr_t base, size_t n);

    // Initialise a region of memory for allocation.
    void markregionfree(uintptr_t base, size_t size);

    // Initialise a region of memory as in use.
    void markregionused(uintptr_t base, size_t size);

    // Initialise the physical memory manager.
    void initialise(mb_info_t *mbd);
}

#endif
