#ifndef FREELSD_KERNEL_MEMORY_VIRTUAL_HEADER
#define FREELSD_KERNEL_MEMORY_VIRTUAL_HEADER

#include <interrupts.hpp>
#include <multiboot.hpp>
#include <stddef.h>
#include <stdint.h>

#define pdeindex(addr) ((addr >> 22) & 0x3FF)  // Returns the 10 bits for a PDE index.
#define pteindex(addr) ((addr >> 12) & 0x3FF)  // Returns the 10 bits for a PTE index.

#define PGE_PAGES_PER_TABLE       1024         // How many page entries per page table.
#define PGE_TABLES_PER_DIRECTORY  1024         // The number of page tables per directory.

// ------------------------------------
// Address ranges of paging structures.
// ------------------------------------

#define PGE_INDIVIDUAL_PAGE_SIZE  0x00001000   // The size of one page (in bytes).
#define PDE_TABLE_ADDR_SIZE       0x00400000   // The address range of a page table.
#define PGE_DIRECTORY_ADDR_SIZE   0xFFFFFFFF   // The address range of a page directory.
#define PGE_FRAME_BITS            0x7FFFF000   // AND with a virtual address to retrieve frame address.

// -----------------------------
// Format of a page table entry.
// -----------------------------

#define PTE_PRESENT_BIT           0x00000001
#define PTE_WRITABLE_BIT          0x00000002
#define PTE_USER_BIT              0x00000004
#define PTE_WRITETHROUGH_BIT      0x00000008
#define PTE_UNCACHEABLE_BIT       0x00000010
#define PTE_ACCESSED_BIT          0x00000020
#define PTE_DIRTY_BIT             0x00000040
#define PTE_PAT_BIT               0x00000080
#define PTE_CPU_GLOBAL_BIT        0x00000100
#define PTE_LV4_GLOBAL_BIT        0x00000200

// ---------------------------------
// Format of a page directory entry.
// ---------------------------------

#define PDE_PRESENT_BIT           0x00000001
#define PDE_WRITABLE_BIT          0x00000002
#define PDE_USER_BIT              0x00000004
#define PDE_PWT_BIT               0x00000008
#define PDE_PCD_BIT               0x00000010
#define PDE_ACCESSED_BIT          0x00000020
#define PDE_DIRTY_BIT             0x00000040
#define PDE_4MB_BIT               0x00000080
#define PDE_CPU_GLOBAL_BIT        0x00000100
#define PDE_LV4_GLOBAL_BIT        0x00000200

extern "C" {
    // Set control register three to a specified address.
    void loadcr3(uint32_t address);
}

namespace mem {
    typedef uint32_t pt_entry_t;  // A page table entry.
    typedef uint32_t pd_entry_t;  // A page directory entry.

    typedef struct table {
        pt_entry_t entries[PGE_PAGES_PER_TABLE];
    } pt_table_t;

    typedef struct directory {
        pd_entry_t entries[PGE_TABLES_PER_DIRECTORY];
    } pd_directory_t;

    // The kernel's page directory.
    extern pd_directory_t *kpdptr;

    // The current page directory in use.
    extern pd_directory_t *currentdir;

    // Page fault exception handler.
    void pfhandler(idt::regs32_t *regs);

    // Undefined exception handler.
    void udhandler(idt::regs32_t *regs);

    // Find the first instance of n pages of memory in a given page directory.
    uintptr_t findfirstfree(pd_directory_t *dir, uintptr_t start, uintptr_t end, size_t n);

    // Map a page-aligned virtual address to a page-aligned physical address.
    void mappage(pd_directory_t *dir, uintptr_t virt, uintptr_t phys);

    // Allocates n virtual pages in a page directory.
    void *allocatevirt(pd_directory_t *dir, uintptr_t start, uintptr_t end, size_t n);

    // Frees n virtual pages (and their physical counterparts) in a page directory.
    void freevirt(pd_directory_t *dir, uintptr_t base, size_t n);

    // Initialise the virtual memory manager.
    void initialisevirt(mb_info_t *mbd);
}

#endif
