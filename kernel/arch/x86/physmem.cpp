#include <mem/phys.hpp>
#include <mem/libc.hpp>
#include <errors.hpp>

namespace physmem {
    static uint32_t map[PMMGR_BITMAP_ARRAY_SIZE];
    size_t usedblocks;
    size_t maxblocks;
    size_t totalsize;

    inline void setbit(size_t index) {
        map[index / 32] |= (1 << (index % 32));
    }

    inline void unsetbit(size_t index) {
        map[index / 32] &= ~(1 << (index % 32));
    }

    inline bool testbit(size_t index) {
        return map[index / 32] & (1 << (index % 32));
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

    void *allocblocks(size_t n) {
        // If not enough blocks available, return.
        if(maxblocks - usedblocks < n) return 0;

        // Find the first free instance.
        int blocks = findfirstfree(n);
        if(blocks == -1) return 0;
        usedblocks += n;

        // Set the required bits in the bitmap and return.
        for(size_t i = 0; i < n; i++) setbit(blocks + i);
        return (void*) (blocks * PMMGR_BLOCK_SIZE);
    }

    void freeblocks(uintptr_t base, size_t n) {
        // Get the block index for unsetting and flip.
        size_t blocks = base / PMMGR_BLOCK_SIZE;
        for(size_t i = 0; i < n; i++) unsetbit(blocks + i);
        usedblocks -= n;
    }

    void markregionfree(uintptr_t base, size_t size) {
        // Get the block's index and number of blocks.
        size_t blocks = size / PMMGR_BLOCK_SIZE + 1;
        size_t align = base / PMMGR_BLOCK_SIZE;

        // Unset and decrement.
        while(blocks-- > 0) {
            unsetbit(align++);
            usedblocks--;
        }
    }

    void markregionused(uintptr_t base, size_t size) {
        // Get the block's index and number of blocks.
        size_t blocks = size / PMMGR_BLOCK_SIZE + 1;
        size_t align = base / PMMGR_BLOCK_SIZE;

        // Set and increment.
        while(blocks-- > 0) {
            setbit(align++);
            usedblocks++;
        }
    }

    void initialise(mb_info_t *mbd) {
        // Panic if bit 0 of multiboot info isn't set.
        if(!checkbit(mbd->flags, 0) || !checkbit(mbd->flags, 6)) panic("grub memory fields invalid!");

        // Grab available memory in kilobytes.
        totalsize = mbd->lowermem + mbd->uppermem;
        maxblocks = totalsize * 1024 / PMMGR_BLOCK_SIZE;

        // Set all memory as in use.
        memset(map, 0xFF, PMMGR_BITMAP_ARRAY_SIZE * 4);
        usedblocks = maxblocks;

        // Iterate through GRUB's memory map.
        mb_mmap_t *mmap = (mb_mmap_t*) mbd->mmapaddr;

        while((uintptr_t) mmap < mbd->mmapaddr + mbd->mmaplength) {
            if(mmap->type == 1) markregionfree(mmap->lowaddr, mmap->lowlen);
            mmap = (mb_mmap_t*) ((uintptr_t) mmap + mmap->size + sizeof(mmap->size));
        }

        // Set the first block as in use so we can use NULL as a bad pointer.
        setbit(0);

        // Mark the kernel and related multiboot structs as in use.
        markregionused(0x100000, (size_t) &kernelend - 0xC0000000);
        markregionused((uintptr_t) mbd, sizeof(mb_info_t));
        markregionused((uintptr_t) mbd->mmapaddr, mbd->mmaplength);
        markregionused((uintptr_t) mbd->modaddr, mbd->modcount * sizeof(mb_modlist_t));

        // Mark loaded GRUB modules as in use.
        mb_modlist_t *mods = (mb_modlist_t*) mbd->modaddr;
        for(uint32_t i = 0; i < mbd->modcount; i++) markregionused(mods[i].modstart, mods[i].modend - mods[i].modstart);
    }
}
