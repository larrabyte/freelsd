#include <multiboot.hpp>
#include <mem/phys.hpp>
#include <mem/libc.hpp>
#include <serial.hpp>

namespace mem {
    static uint64_t physmap[PMMGR_BITMAP_ARRAY_SIZE];
    size_t usedblocks;
    size_t maxblocks;
    size_t totalsize;

    inline void setbit(size_t index) {
        physmap[index / 64] |= (1 << (index % 64));
    }

    inline void unsetbit(size_t index) {
        physmap[index / 64] &= ~(1 << (index % 64));
    }

    inline bool testbit(size_t index) {
        return physmap[index / 64] & (1 << (index % 64));
    }

    int findfirstfree(size_t n) {
        // Iterate through each individual bit in the array.
        for(size_t i = 0, j = 0; i < PMMGR_BITMAP_ARRAY_SIZE; i++) {
            // Is it zero?
            if(!testbit(i)) {
                // Return the appropriate index if we have enough blocks.
                if(++j == n) return (n > 1) ? i - j + 1 : i;
            } else j = 0;  // Not enough zeros.
        }

        return -1;  // No free bits :(
    }

    void *allocatephys(size_t n) {
        // If not enough blocks available, return.
        if(maxblocks - usedblocks < n) return 0;

        // Find the first free instance.
        int blocks = findfirstfree(n);
        if(blocks == -1) return 0;
        usedblocks += n;

        // Set the required bits in the bitmap and return.
        for(size_t i = 0; i < n; i++) setbit(blocks + i);
        return (void*) ((uint64_t) blocks * PMMGR_BLOCK_SIZE);
    }

    void freephys(uintptr_t base, size_t n) {
        // Get the block index for unsetting and flip.
        size_t blocks = base / PMMGR_BLOCK_SIZE;
        for(size_t i = 0; i < n; i++) unsetbit(blocks + i);
        usedblocks -= n;
    }

    void markphysfree(uintptr_t base, size_t size) {
        // Get the block's index and number of blocks.
        size_t blocks = size / PMMGR_BLOCK_SIZE;
        if(size % PMMGR_BLOCK_SIZE != 0) blocks++;
        size_t align = base / PMMGR_BLOCK_SIZE;

        // Unset and decrement.
        while(blocks-- > 0) {
            unsetbit(align++);
            usedblocks--;
        }
    }

    void markphysused(uintptr_t base, size_t size) {
        // Get the block's index and number of blocks.
        size_t blocks = size / PMMGR_BLOCK_SIZE;
        if(size % PMMGR_BLOCK_SIZE != 0) blocks++;
        size_t align = base / PMMGR_BLOCK_SIZE;

        // Set and increment.
        while(blocks-- > 0) {
            setbit(align++);
            usedblocks++;
        }
    }

    void initialisephys(void) {
        // Set all physical memory as in use.
        memset(physmap, 0xFF, PMMGR_BITMAP_ARRAY_SIZE * 8);

        // Start iterating through GRUB's memory map.
        mb_mmap_entry_t *mmapcur = mboot::info.mmap->entries;
        while((uintptr_t) mmapcur < (uintptr_t) mboot::info.mmap->entries + mboot::info.mmap->size - 16) {
            // If we hit an available section of memory, then:
            if(mmapcur->type == MULTIBOOT_MEMORY_AVAILABLE) {
                // Mark it as free and add it to the total memory available.
                markphysfree(mmapcur->addr, mmapcur->len);
                totalsize += mmapcur->len;
            }

            // Advance along the memory map to the next entry.
            mmapcur = (mb_mmap_entry_t*) ((uintptr_t) mmapcur + mboot::info.mmap->entrysize);
        }

        // Properly define the maximum and used number of blocks.
        maxblocks = totalsize / PMMGR_BLOCK_SIZE;
        usedblocks = 0;

        // Mark the kernel and any additional data structures as in use.
        markphysused(0x100000, (size_t) &kernelend - 0x100000);
        markphysused(pge64s, pge64l);
    }
}
