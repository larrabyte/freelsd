#include <multiboot.hpp>
#include <mem/phys.hpp>
#include <mem/virt.hpp>
#include <mem/libc.hpp>
#include <errors.hpp>

namespace mem {
    static uint64_t physmap[PMMGR_BITMAP_ARRAY_SIZE];
    size_t usedblocks;
    size_t maxblocks;
    size_t totalsize;

    inline void setbit(size_t index) {
        physmap[index / 64] |= (1UL << (index % 64));
    }

    inline void unsetbit(size_t index) {
        physmap[index / 64] &= ~(1UL << (index % 64));
    }

    inline bool testbit(size_t index) {
        return physmap[index / 64] & (1UL << (index % 64));
    }

    int findfirstfree(size_t n) {
        // Iterate through each individual bit in the array.
        for(size_t i = 0, j = 0; i < PMMGR_BITMAP_ARRAY_SIZE * 64; i++) {
            // Is it zero?
            if(!testbit(i)) {
                // Return the appropriate index if we have enough blocks.
                if(++j == n) return (n > 1) ? i - j + 1 : i;
            } else j = 0;  // Not enough zeros.
        }

        return -1;  // No free bits :(
    }

    void *allocatephys(size_t n) {
        // Find the first free instance.
        int blocks = findfirstfree(n);
        if(blocks == -1) panic("system tried to allocate %zd physical blocks, %zd remaining.", n, maxblocks - usedblocks);
        usedblocks += n;

        // Set the required bits in the bitmap and return.
        for(size_t i = 0; i < n; i++) setbit(blocks + i);
        return (void*) ((uint64_t) blocks * PMMGR_BLOCK_SIZE);
    }

    void freephys(uintptr_t base, size_t n) {
        // Get the block index for unsetting and flip.
        size_t blocks = base / PMMGR_BLOCK_SIZE;
        if(base % PMMGR_BLOCK_SIZE != 0) blocks++;

        for(size_t i = 0; i < n; i++) unsetbit(blocks + i);
        usedblocks -= n;
    }

    void markphysfree(uintptr_t base, size_t size) {
        // Get the block's index and number of blocks.
        size_t blocks = size / PMMGR_BLOCK_SIZE;
        if(base % PMMGR_BLOCK_SIZE != 0) blocks++;
        size_t align = base / PMMGR_BLOCK_SIZE;

        // Unset and decrement.
        while(blocks-- > 0) {
            if(testbit(align)) {
                unsetbit(align);
                usedblocks--;
            }

            align++;
        }
    }

    void markphysused(uintptr_t base, size_t size) {
        // Get the block's index and number of blocks.
        size_t blocks = size / PMMGR_BLOCK_SIZE;
        if(size % PMMGR_BLOCK_SIZE != 0) blocks++;
        size_t align = base / PMMGR_BLOCK_SIZE;

        // Set and increment.
        while(blocks-- > 0) {
            if(!testbit(align)) {
                setbit(align);
                usedblocks++;
            }

            align++;
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

        // Mark the kernel and the bootstrap paging structures as in use.
        markphysused(0x100000, (size_t) &kernelend - PGE_KERNEL_VBASE);
        markphysused(pge64sel[0], pge64sel[2]);

        // Mark multiboot structures as in use so they aren't corrupted by the PMM.
        markphysused((uintptr_t) mboot::info.fbinfo, mboot::info.fbinfo->common.size);
        markphysused((uintptr_t) mboot::info.meminfo, mboot::info.meminfo->size);
        markphysused((uintptr_t) mboot::info.bootdev, mboot::info.bootdev->size);
        markphysused((uintptr_t) mboot::info.mmap, mboot::info.mmap->size);
    }
}
