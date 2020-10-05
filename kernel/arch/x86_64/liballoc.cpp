#include <mem/alloc.hpp>
#include <mem/virt.hpp>
#include <mem/libc.hpp>

// Durand's Amazing Super Duper Memory functions.

#define LIBALLOC_MAGIC  0xC001C0DE  // Liballoc magic number.
#define LIBALLOC_DEAD   0xDEADDEAD  // Liballoc dead number?
#define VERSION         "1.1"       // Liballoc version.
#define ALIGNMENT       16UL        // The byte alignment of allocated memory.
#define ALIGNTYPE       char        // Pointer alignment type.

// Alignment info is stored right before the pointer. This is the number of bytes of information stored there.
#define ALIGNINFO  sizeof(ALIGNTYPE) * 16  

#define USECASE1
#define USECASE2
#define USECASE3
#define USECASE4
#define USECASE5

static liballoc_major_t *memroot = nullptr;  // The root memory block acquired from the system.
static liballoc_major_t *bestbet = nullptr;  // The major with the most free memory.

static uint64_t pagesize = 4096;  // The size of an individual page in bytes.
static uint64_t pagecount = 16;   // The number of pages to request per chunk.
static uint64_t allocated = 0;    // The running total of allocated memory.
static uint64_t inuse = 0;        // The running total of used memory.

uint64_t liballoc_warnings = 0;   // The number of warnings encountered.
uint64_t liballoc_errorcnt = 0;   // The number of actual errors.
uint64_t liballoc_poverrun = 0;   // The number of possible overruns.

static int lock(void) {
    asm volatile("cli");
    return 0;
}

static int unlock(void) {
    asm volatile("sti");
    return 0;
}

static void *allocate(size_t n) {
    return mem::allocatevirt(mem::getkernelpml4(false), LIBALLOC_KERNEL_HEAP_START, LIBALLOC_KERNEL_HEAP_FINAL, n);
}

static int release(void *pointer, size_t n) {
    mem::freevirt(mem::getkernelpml4(false), (uintptr_t) pointer, n);
    return 0;
}

static inline void *align(void *pointer) {
    if(ALIGNMENT > 1) {
        pointer = (void*) ((uintptr_t) pointer + ALIGNINFO);
        uintptr_t diff = (uintptr_t) pointer & (ALIGNMENT - 1);

        if(diff != 0) {
            diff = ALIGNMENT - diff;
            pointer = (void*) ((uintptr_t) pointer + diff);
        }

        *((ALIGNTYPE*) ((uintptr_t) pointer - ALIGNINFO)) = diff + ALIGNINFO;
    }

    return pointer;
}

static inline void *unalign(void *pointer) {
    if(ALIGNMENT > 1) {
        uintptr_t diff = *((ALIGNTYPE*) ((uintptr_t) pointer - ALIGNINFO));
        if(diff < (ALIGNMENT + ALIGNINFO)) pointer = (void*) ((uintptr_t) pointer - diff);
    }

    return pointer;
}

static liballoc_major_t *allocnewpage(size_t size) {
    liballoc_major_t *maj;
    size_t st, reqbytes;

    // This is how much space is required.
    reqbytes = size + sizeof(liballoc_major_t) + sizeof(liballoc_minor_t);

    st = reqbytes / pagesize;
    if(reqbytes % pagesize != 0) st++;

    // Make sure it's >= the minimum size.
    if(st < pagecount) st = pagecount;
    maj = (liballoc_major_t*) allocate(st);

    // Uh oh, we ran out of memory.
    if(maj == nullptr) {
        liballoc_warnings += 1;
        return nullptr;
    }

    maj->prev = nullptr;
    maj->next = nullptr;
    maj->pages = st;
    maj->size = st * pagesize;
    maj->usage = sizeof(liballoc_major_t);
    maj->first = nullptr;

    allocated += maj->size;
    return maj;
}

void *kmalloc(size_t reqsize) {
    uint64_t startedbet = 0, bestsize = 0;
    size_t size = reqsize;
    uintptr_t diff;
    void *p = nullptr;

    liballoc_minor_t *min, *newmin;
    liballoc_major_t *maj;

    // For alignment, we adjust size so there's enough space to align.
    if(ALIGNMENT > 1) size += ALIGNMENT + ALIGNINFO;

    lock();

    if(size == 0) {
        liballoc_warnings += 1;
        unlock();
        return kmalloc(1);
    }

    // This is the first time we are being used.
    if(memroot == nullptr) {
        memroot = allocnewpage(size);

        // Return nullptr if we can't initialise.
        if(memroot == nullptr) {
            unlock();
            return nullptr;
        }
    }

    maj = memroot;

    // Start at the best bet.
    if(bestbet != nullptr) {
        bestsize = bestbet->size - bestbet->usage;
        if(bestsize > (size + sizeof(liballoc_minor_t))) {
            maj = bestbet;
            startedbet = 1;
        }
    }

    while(maj != nullptr) {
        // Get free memory in the block.
        diff = maj->size - maj->usage;

        // Hmm, this one has more memory then our best bet. Remember!
        if(bestsize < diff) {
            bestbet = maj;
            bestsize = diff;
        }

        #ifdef USECASE1

        // Case 1: There isn't enough space in this major block.
        if(diff < (size + sizeof(liballoc_minor_t))) {

            if(maj->next != nullptr) {
                maj = maj->next;     // Set the new maj to the next one.
                continue;            // Hop to that one.
            }

            // If we started at the best bet, start over again.
            if(startedbet == 1) {
                maj = memroot;
                startedbet = 0;
                continue;
            }

            // Create a new major block next to this one.
            maj->next = allocnewpage(size);
            if(maj->next == nullptr) break;
            maj->next->prev = maj;
            maj = maj->next;

            // Fall through to case 2...
        }

        #endif

        #ifdef USECASE2

        // Case 2: It's a brand new block.
        if(maj->first == nullptr) {
            maj->first = (liballoc_minor_t*) ((uintptr_t) maj + sizeof(liballoc_major_t));

            maj->usage += size + sizeof(liballoc_minor_t);
            maj->first->magic = LIBALLOC_MAGIC;
            maj->first->reqsize = reqsize;
            maj->first->prev = nullptr;
            maj->first->next = nullptr;
            maj->first->block = maj;
            maj->first->size = size;

            inuse += size;

            p = (void*) ((uintptr_t) maj->first + sizeof(liballoc_minor_t));
            p = align(p);

            // Release the lock and return.
            unlock();
            return p;
        }

        #endif

        #ifdef USECASE3

        // Case 3: Block in use and enough space at the start of the block.
        diff = (uintptr_t) maj->first - (uintptr_t) maj - sizeof(liballoc_major_t);
        if(diff >= (size + sizeof(liballoc_minor_t))) {
            // Yes, space in front. Squeeze in.
            maj->first->prev = (liballoc_minor_t*) ((uintptr_t) maj + sizeof(liballoc_major_t));
            maj->first->prev->next = maj->first;
            maj->first = maj->first->prev;

            maj->first->magic = LIBALLOC_MAGIC;
            maj->first->prev = nullptr;
            maj->first->block = maj;
            maj->first->size = size;
            maj->first->reqsize = reqsize;
            maj->usage += size + sizeof(liballoc_minor_t);

            inuse += size;

            p = (void*) ((uintptr_t) maj->first + sizeof(liballoc_minor_t));
            p = align(p);

            // Release the lock and return.
            unlock();
            return p;
        }

        #endif

        #ifdef USECASE4

        // Case 4: There is enough space in this block. But is it contiguous?
        min = maj->first;

        // Looping within the block now...
        while(min != nullptr) {
            // Case 4.1: End of minors in a block. Space from last and end?
            if(min->next == nullptr) {
                // The rest of this block is free... is it big enough?
                diff = (uintptr_t) maj + maj->size - (uintptr_t) min - sizeof(liballoc_minor_t) - min->size;

                if(diff >= (size + sizeof(liballoc_minor_t))) {
                    // Yay...
                    min->next = (liballoc_minor_t*) ((uintptr_t) min + sizeof(liballoc_minor_t) + min->size);
                    min->next->prev = min;
                    min = min->next;
                    min->next = nullptr;
                    min->magic = LIBALLOC_MAGIC;
                    min->block = maj;
                    min->size = size;
                    min->reqsize = reqsize;
                    maj->usage += size + sizeof(liballoc_minor_t);

                    inuse += size;

                    p = (void*) ((uintptr_t) min + sizeof(liballoc_minor_t));
                    p = align(p);

                    // Release the lock and return.
                    unlock();
                    return p;
                }
            }

            // Case 4.2: Is there space between two minors?
            if(min->next != nullptr) {
                // Is the difference between here and next big enough?
                diff = (uintptr_t) min->next - (uintptr_t) min - sizeof(liballoc_minor_t) - min->size;
        
                if(diff >= (size + sizeof(liballoc_minor_t))) {
                    // Yay...
                    newmin = (liballoc_minor_t*) ((uintptr_t) min + sizeof(liballoc_minor_t) + min->size);

                    newmin->magic = LIBALLOC_MAGIC;
                    newmin->next = min->next;
                    newmin->prev = min;
                    newmin->size = size;
                    newmin->reqsize = reqsize;
                    newmin->block = maj;
                    min->next->prev = newmin;
                    min->next = newmin;
                    maj->usage += size + sizeof(liballoc_minor_t);

                    inuse += size;

                    p = (void*) ((uintptr_t) newmin + sizeof(liballoc_minor_t));
                    p = align(p);

                    // Release the lock and return.
                    unlock();
                    return p;
                }
            }

            min = min->next;
        }

        #endif

        #ifdef USECASE5

        // Case 5: block full! Ensure next block and loop.
        if(maj->next == nullptr) {
            if(startedbet == 1) {
                maj = memroot;
                startedbet = 0;
                continue;
            }

            // We've run out. We need more...
            maj->next = allocnewpage(size);
            if(maj->next == nullptr) break;
            maj->next->prev = maj;
        }

        #endif

        maj = maj->next;
    }

    // Release the lock and return.
    unlock();
    return nullptr;
}

void kfree(void *pointer) {
    liballoc_major_t *maj;
    liballoc_minor_t *min;

    if(pointer == nullptr) {
        liballoc_warnings += 1;
        return;
    }

    pointer = unalign(pointer);
    lock(); // Lock it.

    min = (liballoc_minor_t*) ((uintptr_t) pointer - sizeof(liballoc_minor_t));

    if(min->magic != LIBALLOC_MAGIC) {
        liballoc_errorcnt += 1;

        // Check for overrun errors (all bytes of LIBALLOC_MAGIC)
        if(((min->magic & 0xFFFFFF) == (LIBALLOC_MAGIC & 0xFFFFFF)) ||
           ((min->magic & 0x00FFFF) == (LIBALLOC_MAGIC & 0x00FFFF)) ||
           ((min->magic & 0x0000FF) == (LIBALLOC_MAGIC & 0x0000FF))) {

            liballoc_poverrun += 1;
        }

        unlock();
        return;
    }

    maj = min->block;
    inuse -= min->size;

    maj->usage -= (min->size + sizeof(liballoc_minor_t));
    min->magic = LIBALLOC_DEAD; // No mojo.

    if(min->next != nullptr) min->next->prev = min->prev;
    if(min->prev != nullptr) min->prev->next = min->next;
    if(min->prev == nullptr) maj->first = min->next;

    // We need to clean up after the majors now.
    if(maj->first == nullptr) { // Block completely unused.
        if(memroot == maj) memroot = maj->next;
        if(bestbet == maj) bestbet = nullptr;
        if(maj->prev != nullptr) maj->prev->next = maj->next;
        if(maj->next != nullptr) maj->next->prev = maj->prev;
        allocated -= maj->size;

        release(maj, maj->pages);
    } else {
        if(bestbet != nullptr) {
            unsigned int bestsize = bestbet->size - bestbet->usage;
            unsigned int majsize = maj->size - maj->usage;

            if(majsize > bestsize) bestbet = maj;
        }
    }

    // Release the lock.
    unlock();
}

void *kcalloc(size_t objcount, size_t objsize) {
    size_t memsize = objcount * objsize;
    void *p = kmalloc(memsize);
    memset(p, 0, memsize);

    return p;
}

void *krealloc(void *pointer, size_t size) {
    liballoc_minor_t *min;
    unsigned int realsize;
    void *p;

    // Honour the case of size == 0 => free old and return nullptr.
    if(size == 0) {
        kfree(pointer);
        return nullptr;
    }

    // In the case of a nullptr pointer, return a simple malloc.
    if(pointer == nullptr) return kmalloc(size);

    // Unalign the pointer if required.
    p = unalign(pointer);
    lock();

    min = (liballoc_minor_t*) ((uintptr_t) pointer - sizeof(liballoc_minor_t));

    // Ensure it's a valid structure.
    if(min->magic != LIBALLOC_MAGIC) {
        liballoc_errorcnt += 1;

        if(((min->magic & 0xFFFFFF) == (LIBALLOC_MAGIC & 0xFFFFFF)) ||
           ((min->magic & 0x00FFFF) == (LIBALLOC_MAGIC & 0x00FFFF)) ||
           ((min->magic & 0x0000FF) == (LIBALLOC_MAGIC & 0x0000FF))) {

            liballoc_poverrun += 1;
        }

        // Being lied to... release the lock.
        unlock();
        return nullptr;
    }

    // Definitely a memory block.
    realsize = min->reqsize;

    if(realsize >= size) {
        min->reqsize = size;
        unlock();
        return pointer;
    }

    unlock();

    // If we got here than we're reallocating to a block bigger than us.
    p = kmalloc(size);
    memcpy(p, pointer, realsize);
    kfree(pointer);

    return p;
}
