#ifndef FREELSD_KERNEL_MEMORY_PHYSICAL_HEADER
#define FREELSD_KERNEL_MEMORY_PHYSICAL_HEADER

#include <multiboot.hpp>
#include <stdint.h>
#include <stddef.h>

#define PMMGR_BITMAP_ARRAY_SIZE  32768  // The size of the bitmap array (in dwords).
#define PMMGR_BLOCK_SIZE         4096   // The size of a single block (in bytes).
#define PMMGR_BLOCKS_PER_BYTE    8      // The number of blocks managed by one byte.

extern "C" void *kernelend;

namespace mem {
    extern size_t usedblocks;  // Number of used physical blocks.
    extern size_t maxblocks;   // Total number of physical blocks available.
    extern size_t totalsize;   // Total physical memory available (in kilobytes).

    // Allocate n physical blocks.
    void *allocatephys(size_t n);

    // Free n physical blocks, starting at base.
    void freephys(uintptr_t base, size_t n);

    // Initialise a region of memory for allocation.
    void markphysfree(uintptr_t base, size_t size);

    // Initialise a region of memory as in use.
    void markphysused(uintptr_t base, size_t size);

    // Initialise the physical memory manager.
    void initialisephys(mb_info_t *mbd);
}

#endif