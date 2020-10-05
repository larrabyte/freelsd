#ifndef FREELSD_KERNEL_MEMORY_PHYSICAL_HEADER
#define FREELSD_KERNEL_MEMORY_PHYSICAL_HEADER

#include <stdint.h>
#include <stddef.h>

#define PMMGR_MAX_BLOCK_COUNT    16777216  // The maximum number of blocks using the bitmap.
#define PMMGR_BITMAP_ARRAY_SIZE  262144    // The size of the bitmap array (in qwords).
#define PMMGR_BLOCK_SIZE         4096      // The size of a single block (in bytes).
#define PMMGR_BLOCKS_PER_BYTE    8         // The number of blocks managed by one byte.
  
namespace mem {
    typedef struct bootdata {
        uint64_t start;
        uint64_t end;
        uint64_t size;
    } bootdata_t;

    extern size_t usedblocks;  // Number of used physical blocks.
    extern size_t maxblocks;   // Total number of physical blocks available.
    extern size_t totalsize;   // Total physical memory available in bytes.

    // Find the first free instance of n contiguous physical blocks.
    int findfirstfree(size_t n);

    // Allocate n physical blocks.
    void *allocatephys(size_t n);

    // Free n physical blocks, starting at base.
    void freephys(uintptr_t base, size_t n);

    // Initialise a region of memory for allocation.
    void markphysfree(uintptr_t base, size_t size);

    // Initialise a region of memory as in use.
    void markphysused(uintptr_t base, size_t size);

    // Initialise the physical memory manager.
    void initialisephys(void);
}

#endif
