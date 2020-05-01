#include <mem/virt.hpp>
#include <mem/phys.hpp>
#include <mem/libc.hpp>
#include <errors.hpp>

/*  Paging information, formats and data structures.
    ------------------------------------------------
    Page directories and table layouts:
        pg_directory_t:       --> Manages 4GB address space.
            pd_entry_t[1024]  --> 1024 page directory entries.

        pd_entry_t:           --> Manages 4MB address space.
            pg_table_t*       --> Address of a page table (along with flags)

        pg_table_t:           --> Manages 4MB address space.
            pt_entry_t[1024]  --> 1024 page table entries.

        pt_entry_t:           --> Manages 4KB address space.
            uint32_t          --> Address of physical memory (along with flags)

    Format of a virtual address:
        0xAAAAAAAAAA BBBBBBBBBB CCCCCCCCCCCC
            A: Directory index       (bits 22-32)
            B: Page table index      (bits 12-22)
            C: Offset into the page  (bits 00-12)
*/

namespace virtmem {
    pd_directory_t *kernelpd, *currentdir;

    inline void addattribute(uint32_t *entry, uint32_t attribute) {
        *entry |= attribute;
    }

    inline void delattribute(uint32_t *entry, uint32_t attribute) {
        *entry &= ~attribute;
    }

    inline bool testattribute(uint32_t *entry, uint32_t attribute) {
        return *entry & attribute;
    }

    inline void setframe(uint32_t *entry, uint32_t physaddr) {
        *entry = (*entry & ~PGE_FRAME_BITS) | physaddr;
    }

    inline uint32_t getframeaddr(uint32_t *entry) {
        return *entry & ~0xFFF;
    }

    void pfhandler(idt::regs32_t *regs) {
        // Fetch the page fault address from control register 2.
        uint32_t faulty; asm volatile("mov %%cr2, %0" : "=r" (faulty));

        panic("page fault exception generated.\n"
              "[kernel] fault address (eip): %p\n"
              "[kernel] page no-execute bit: %s\n"
              "[kernel] page supervisor bit: %s\n"
              "[kernel] page writable bit: %s\n"
              "[kernel] page reserved bit: %s\n"
              "[kernel] page present bit: %s", faulty,
              (checkbit(regs->errcode, 4)) ? "set" : "unset",
              (checkbit(regs->errcode, 2)) ? "set" : "unset",
              (checkbit(regs->errcode, 1)) ? "set" : "unset",
              (checkbit(regs->errcode, 3)) ? "set" : "unset",
              (checkbit(regs->errcode, 0)) ? "set" : "unset");
    }

    void udhandler(idt::regs32_t *regs) {
        uint32_t faulty; asm volatile("mov %%cr2, %0" : "=r" (faulty));

        panic("invalid opcode, %s\n"
              "[kernel] printing register dump to display and serial.\n\n"
              "[kernel]  gs: %p,  fs: %p,  es: %p,  ds: %p\n"
              "[kernel] edi: %p, esi: %p, ebp: %p, esp: %p\n"
              "[kernel] ebx: %p, edx: %p, ecx: %p, eax: %p\n"
              "[kernel] eip: %p, cr2: %p,  cs: %p,  ss: %p",
              (faulty == 0x0) ? "attempted NULL dereference." : "unknown cause.",
              regs->gs, regs->fs, regs->es, regs->ds, regs->edi, regs->esi, regs->ebp,
              regs->esp, regs->ebx, regs->edx, regs->ecx, regs->eax, regs->eip, faulty, regs->cs, regs->ss);
    }

    uintptr_t findfirstfree(pd_directory_t *directory, uintptr_t start, uintptr_t end, size_t n) {
        if(n > PGE_PAGES_PER_TABLE || n == 0) return 0;
        uintptr_t retaddr = start;
        uintmax_t freepages = 0;

        for(uint32_t i = pdeindex(start); i < pdeindex(end); i++) {
            // Return the PDE's index if it's not present.
            pd_entry_t *pde = &directory->entries[i];
            if(!checkbit(*pde, 0)) return (i << 22);

            // Otherwise, it's present, get the table's address.
            pt_table_t *table = (pt_table_t*) getframeaddr(pde);

            // Iterate through each page table entry.
            for(int j = 0; j < PGE_PAGES_PER_TABLE; j++) {
                // If the PTE is present, skip this loop iteration.
                if(checkbit(table->entries[j], 0)) {
                    retaddr += 0x1000; continue;
                } else if(++freepages == n) return retaddr;
            }
        }

        return 0;
    }

    void *allockernelheap(size_t n) {
        // The kernel heap is given 256MB to work with.
        uintptr_t virt = findfirstfree(kernelpd, 0xD0000000, 0xE0000000, n);
        uintptr_t phys = (uintptr_t) physmem::allocblocks(n);
        uintptr_t max = virt + (n * PMMGR_BLOCK_SIZE);

        if(virt == 0) panic("kernel has run out of virtual heap memory.");
        else if(phys == 0) panic("system has run out of physical memory.");

        // Map our free virtual address space to some physical blocks of memory.
        for(uintptr_t virtaddr = virt; virtaddr < max; phys += 0x1000, virtaddr += 0x1000) {
            mappage(virtaddr, phys);
        }

        return (void*) virt;
    }

    void freekernelheap(uintptr_t base, size_t n) {
        pd_entry_t *pde = &kernelpd->entries[pdeindex(base)];
        if(n > PGE_PAGES_PER_TABLE || n == 0) return;

        // Iterate through each entry and free each physical block.
        pt_table_t *table = (pt_table_t*) getframeaddr(pde);
        for(uint32_t i = pteindex(base); i < pteindex(base) + n; i++) {
            pt_entry_t *pte = &table->entries[i];
            physmem::freeblocks(getframeaddr(pte), 1);
            delattribute(pte, PTE_PRESENT_BIT | PTE_WRITABLE_BIT);
        }

        // If we want to free the whole table, free the directory entry as well.
        if(n == PGE_PAGES_PER_TABLE) {
            physmem::freeblocks((uintptr_t) table, 1);
            delattribute(pde, PDE_PRESENT_BIT | PDE_WRITABLE_BIT);
        }
    }

    void mappage(uintptr_t virt, uintptr_t phys) {
        // Get the page directory entry associated with the virtual address.
        pd_entry_t *pde = &currentdir->entries[pdeindex(virt)];
        pt_table_t *table;

        // Does the directory have the present bit set?
        if(!checkbit(*pde, 0)) {
            // No, allocate a page table and zero it out.
            table = (pt_table_t*) physmem::allocblocks(1);
            if(table) memset(table, 0, sizeof(pt_table_t));
            else return;

            // Map the new page table into our page directory entry.
            addattribute(pde, PDE_PRESENT_BIT | PDE_WRITABLE_BIT);
            setframe(pde, (uint32_t) table);
        } else {
            // Yes, fetch the address for the page table.
            table = (pt_table_t*) getframeaddr(pde);
        }

        // Fetch the address of the page table entry and map it in.
        pt_entry_t *page = &table->entries[pteindex(virt)];
        addattribute(page, PTE_PRESENT_BIT | PTE_WRITABLE_BIT);
        setframe(page, phys);
    }

    void initialise(mb_info_t *mbd) {
        // Allocate a kernel page directory and zero it out.
        kernelpd = (pd_directory_t*) physmem::allocblocks(1);
        memset(kernelpd, 0, sizeof(pd_directory_t));
        currentdir = kernelpd;

        // Identity map the first megabyte of physical memory.
        for(uintptr_t firstmb = 0x0; firstmb < 0x100000; firstmb += 0x1000) mappage(firstmb, firstmb);

        // Map sixteen megabytes from 0x100000 to 0xC0000000 (virtual address of 3072MB).
        for(uintptr_t kernelphys = 0x100000; kernelphys < 0x1000000; kernelphys += 0x1000) mappage(kernelphys + 0xC0000000, kernelphys);

        // Map sixty four megabytes of the framebuffer to 0xFC00000000 (virtual address of 4032MB).
        for(uintptr_t framephys = mbd->framebufferaddr, framevirt = 0xFC000000; framevirt < 0xFFFFF000; framephys += 0x1000, framevirt += 0x1000) mappage(framevirt, framephys);
        mbd->framebufferaddr = 0xFC000000;

        // Cache the module list pointer and the virtual module load address.
        mb_modlist_t *mods = (mb_modlist_t*) mbd->modaddr;
        uintptr_t modulevirt = 0xD0000000;

        // Map the initrd to the beginning of the kernel heap.
        for(uintptr_t modulephys = mods->modstart; modulephys < mods->modend; modulephys += 0x1000, modulevirt += 0x1000) mappage(modulevirt, modulephys);
        mods->modend = 0xD0000000 + (mods->modend - mods->modstart);
        mods->modstart = 0xD0000000;

        // Register interrupt handlers and switch the active page directory.
        idt::registerhandler(14, &pfhandler);
        idt::registerhandler(6, &udhandler);
        loadcr3((uint32_t) currentdir);
    }
}
