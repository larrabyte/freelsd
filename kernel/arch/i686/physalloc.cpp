#include <mem/physalloc.hpp>
#include <gfx/renderer.hpp>
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

    int getfirstfree(size_t n) {
        if(n <= 0) return -1;

        // Iterate through 32-bit chunks of the bitmap.
        for(size_t i = 0; i < maxblocks; i++) {
            // If all bits are set, no free chunks here.
            if(map[i] == 0xFFFFFFFF) continue;

            // Iterate through the individual bits.
            for(size_t j = 0; j < 32; i++) {

                // We found the first free bit!
                if(!checkbit(map[i], j)) {
                    // If one block requested, return free.
                    if(n == 1) return 32 * i + j;
                    size_t starting = i * 32 + (1 << j);
                    size_t free = 0;

                    // Check if successive bits are also free.
                    for(size_t k = 0; k < n; k++) {
                        if(!testbit(starting + k)) free++;
                        if(free == n) return 32 * i + j;
                    }
                }
            }
        }

        return -1;
    }

    void *allocblocks(size_t n) {
        if(maxblocks - usedblocks < n) return 0;
 
        int block = getfirstfree(n);
        if(block == -1) return 0;
        for(size_t i = 0; i < n; i++) setbit(block + i);
        usedblocks += n;

        return (void*) (block * PMMGR_BLOCK_SIZE);
    }

    void freeblocks(const void *base, size_t size) {
        size_t block = (uintptr_t) base / PMMGR_BLOCK_SIZE;
        for(size_t i = 0; i < size; i++) {
            unsetbit(block + i);
        }

        usedblocks -= size;
    }

    void markregionfree(const void *base, size_t size) {
        size_t align = (uintptr_t) base / PMMGR_BLOCK_SIZE;
        size_t blocks = size / PMMGR_BLOCK_SIZE;

        while(blocks-- > 0) {
            unsetbit(align++);
            usedblocks--;
        }

        // Always set the first block as in use.
        // This ensures that allocations can't be zero.
        setbit(0);
    }

    void markregionused(const void *base, size_t size) {
        size_t align = (uintptr_t) base / PMMGR_BLOCK_SIZE;
        size_t blocks = size / PMMGR_BLOCK_SIZE;

        while(blocks-- > 0) {
            setbit(align++);
            usedblocks++;
        }
    }

    void initialise(mb_info_t *mbd) {
        // Panic if bit 0 of multiboot info isn't set.
        if(!checkbit(mbd->flags, 0) || !checkbit(mbd->flags, 6)) panic("grub memory fields invalid!");

        // Grab available memory in kilobytes.
        totalsize = (mbd->lowermem + mbd->uppermem);
        maxblocks = totalsize * 1024 / PMMGR_BLOCK_SIZE;

        // Set all memory as in use.
        memset(map, 0xFF, maxblocks / PMMGR_BLOCKS_PER_BYTE);
        usedblocks = maxblocks;

        // Iterate through GRUB's memory map.
        mb_mmap_t* mmap = (mb_mmap_t*) mbd->mmapaddr;
        size_t max = mbd->mmaplength / sizeof(mb_mmap_t);

        for(size_t i = 0; i < max; i++) {
            gfx::printf("[physmm] memory map, region %d: starts at 0x%p\n", i, mmap[i].lowaddr);
            gfx::printf("         length of 0x%p, type %d\n\n", mmap[i].lowlen, mmap[i].type);
            if(mmap[i].type == 1) markregionfree((void*) mmap[i].lowaddr, mmap[i].lowlen);
        }
    }
}
