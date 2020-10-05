#ifndef FREELSD_KERNEL_MEMORY_VIRTUAL_HEADER
#define FREELSD_KERNEL_MEMORY_VIRTUAL_HEADER

/*  Paging information and data structures.
    ---------------------------------------
    Paging data structure layout:
        pge_structure_t:       --> Paging abstraction.
            pml4_entry_t[512]  --> 512 page map level 4 entries.            -> 512GB * 512 = 256TB of address space.
            pdp_entry_t[512]   --> 512 page directory pointer entries.      -> 1GB   * 512 = 512GB of address space.
            pd_entry_t[512]    --> 512 page directory entries.              -> 2MB   * 512 = 1GB of address space. 
            pt_entry_t[512]    --> 512 page table entries.                  -> 4KB   * 512 = 2MB of address space.

    Format of a virtual address:
        0xAAAAAAAAAAAAAAAA BBBBBBBBB CCCCCCCCC DDDDDDDDD EEEEEEEEE FFFFFFFFFFFF
            A: Sign extension of bit 47       (bits 48-63)
            B: Page Map Level 4 offset        (bits 39-47)
            C: Page Directory Pointer offset  (bits 30-38)
            D: Page Directory offset          (bits 21-29)
            E: Page table offset              (bits 12-20)
            F: Physical address offset        (bits 00-11)
*/

#include <stddef.h>
#include <stdint.h>

#define pml4index(addr) ((addr >> 39) & 0x1FF)  // Returns the 9 bits for a PML4 index.
#define pdpeindex(addr) ((addr >> 30) & 0x1FF)  // Returns the 9 bits for a PDPT index.
#define pdeindex(addr)  ((addr >> 21) & 0x1FF)  // Returns the 9 bits for a PDE index.
#define pteindex(addr)  ((addr >> 12) & 0x1FF)  // Returns the 9 bits for a PTE index.

#define PGE_KERNEL_VBASE           0xFFFFFFFF80000000
#define PGE_SIGNEXTENSION_BIT      0x0000800000000000
#define PGE_ENTRIES_PER_STRUCTURE  512

#define PGE_MMIO_BASEADDR          0xFFFFFFFF7FE00000
#define PGE_MMIO_ENDADDR           0xFFFFFFFF80000000
#define PGE_MMIO_ADDRSPACE         0x0000000000200000

// ------------------------------------
// Address ranges of paging structures.
// ------------------------------------

#define PGE_PTE_ADDRSPACE          0x0000000000001000  // The address range of a page table entry.
#define PGE_PDE_ADDRSPACE          0x0000000000200000  // The address range of a page directory entry.
#define PGE_PDPE_ADDRSPACE         0x0000000040000000  // The address range of a page pointer table entry.
#define PGE_PML4E_ADDRSPACE        0x0000008000000000  // The address range of a Page Map Level 4 entry.

// --------------------------------------------
// Common control bits of a PTE/PDE/PTPE/PML4E.
// --------------------------------------------

#define PGE_PRESENT_BIT            0x0000000000000001
#define PGE_WRITABLE_BIT           0x0000000000000002
#define PGE_USER_BIT               0x0000000000000004
#define PGE_WRITETHROUGH_BIT       0x0000000000000008
#define PGE_UNCACHEABLE_BIT        0x0000000000000010
#define PGE_ACCESSED_BIT           0x0000000000000020
#define PGE_DIRTY_BIT              0x0000000000000040
#define PGE_HUGEPAGE_BIT           0x0000000000000080
#define PGE_ALLOCATED_BIT          0x0000000000000200
#define PGE_NOEXECUTE_BIT          0x8000000000000000
#define PGE_FRAME_BITS             0x000FFFFFFFFFF000

typedef enum mempagetypes {
    PGE_REGULAR_PAGE = 0x1000,
    PGE_HUGE2MB_PAGE = 0x200000,
    PGE_HUGE1GB_PAGE = 0x40000000
} mem_pagetype_t;

namespace mem {
    typedef uint64_t pge_entry_t;

    typedef struct pgestructure {
        pge_entry_t entries[PGE_ENTRIES_PER_STRUCTURE];
    } pge_structure_t;

    // The current PML4 table in use.
    extern pge_structure_t *currentpml4;

    // Return the address of the kernel's PML4.
    pge_structure_t *getkernelpml4(bool phys);

    // Find the first instance of n pages of memory in a given PML4 table.
    uintptr_t findfirstfree(pge_structure_t *dir, uintptr_t start, uintptr_t end, size_t n);

    // Convert a virtual address in a given PML4 table to a physical address.
    uintptr_t virt2phys(pge_structure_t *pml4, uintptr_t virt);

    // Maps a page-aligned virtual address to a page-aligned physical address.
    void mappage(pge_structure_t *pml4, mem_pagetype_t type, uintptr_t virt, uintptr_t phys, uint64_t flags);

    // Frees a mapped page, starting from base. Will also call mem::freephys() if page was allocated.
    void unmappage(pge_structure_t *pml4, uintptr_t base, size_t n);

    // Allocates n virtual pages in a PML4 table.
    void *allocatevirt(pge_structure_t *pml4, uintptr_t start, uintptr_t end, size_t n);

    // Allocate virtual address space for MMIO in the kernel's PML4.
    void *allocatemmio(uintptr_t phys, size_t n);

    // Frees n virtual pages (and their physical counterparts) in a PML4 table.
    void freevirt(pge_structure_t *pml4, uintptr_t base, size_t n);

    // Initialise the virtual memory manager.
    void initialisevirt(void);
}

#endif
