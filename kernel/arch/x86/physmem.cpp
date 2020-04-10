#include <mem/phys.hpp>
#include <mem/libc.hpp>
#include <errors.hpp>
#include <serial.hpp>

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
        // Iterate through 32-bit chunks of the bitmap.
        for(size_t i = 0; i < maxblocks; i++) {
            if(map[i] == 0xFFFFFFFF) continue;

            // Iterate through the individual bits.
            for(size_t j = 0; j < 32; j++) {

                // We found the first free bit!
                if(!checkbit(map[i], j)) {
                    // If one block requested, return.
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

        return -1;  // No free bits :(
    }

    void *allocblocks(size_t n) {
        if(maxblocks - usedblocks < n) return 0;
        int blocks = findfirstfree(n);
        if(blocks == -1) return 0;
        usedblocks += n;

        for(size_t i = 0; i < n; i++) setbit(blocks + i);
        return (void*) (blocks * PMMGR_BLOCK_SIZE);
    }

    void freeblocks(uintptr_t base, size_t n) {
        size_t blocks = base / PMMGR_BLOCK_SIZE;
        for(size_t i = 0; i < n; i++) unsetbit(blocks + i);
        usedblocks -= n;
    }

    void markregionfree(uintptr_t base, size_t size) {
        size_t blocks = size / PMMGR_BLOCK_SIZE;
        size_t align = base / PMMGR_BLOCK_SIZE;

        while(blocks-- > 0) {
            unsetbit(align++);
            usedblocks--;
        }

        // Always set the first block as in use.
        // This ensures that allocations can't be zero.
        setbit(0);
    }

    void markregionused(uintptr_t base, size_t size) {
        size_t blocks = size / PMMGR_BLOCK_SIZE;
        size_t align = base / PMMGR_BLOCK_SIZE;

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
        size_t memregion = 0;
        serial::printf("\n");

        while((uintptr_t) mmap < mbd->mmapaddr + mbd->mmaplength) {
            serial::printf("[physmm] memory map, region %d: 0x%p (start), 0x%p or %dKB (size), type %d\n",
                           memregion++, mmap->lowaddr, mmap->lowlen, mmap->lowlen / 1024, mmap->type);

            if(mmap->type == 1) markregionfree(mmap->lowaddr, mmap->lowlen);
            mmap = (mb_mmap_t*) ((uintptr_t) mmap + mmap->size + sizeof(mmap->size));
        }

        // Mark the kernel as in use so the PMM doesn't allocate it.
        markregionused(0x100000, (size_t) &kernelend - 0x100000);
    }
}
