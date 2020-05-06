#include <mem/alloc.hpp>
#include <mem/virt.hpp>
#include <mem/phys.hpp>
#include <mem/libc.hpp>
#include <errors.hpp>
#include <serial.hpp>

namespace mem {
    static pt_table_t kernelpt[PGE_TABLES_PER_DIRECTORY] __attribute__((aligned(4096)));
    static pd_directory_t kernelpd __attribute__((aligned(4096)));

    pd_directory_t *kpdphys = (pd_directory_t*) ((char*) &kernelpd - 0xC0000000);
    pd_directory_t *kpdvirt = &kernelpd;
    pd_directory_t *currentdir;

    static inline void addattribute(uint32_t *entry, uint32_t attribute) {
        *entry |= attribute;
    }

    static inline void delattribute(uint32_t *entry, uint32_t attribute) {
        *entry &= ~attribute;
    }

    static inline bool testattribute(uint32_t *entry, uint32_t attribute) {
        return *entry & attribute;
    }

    static inline void setframe(uint32_t *entry, uint32_t physaddr) {
        *entry = (*entry & ~PGE_FRAME_BITS) | physaddr;
    }

    static inline uint32_t getframeaddr(uint32_t *entry) {
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

    uintptr_t findfirstfree(pd_directory_t *dir, uintptr_t start, uintptr_t end, size_t n) {
        if(n == 0) return findfirstfree(dir, start, end, 1);
        else if(dir == 0 || start == 0 || end == 0) return 0;

        bool addrcached = false;
        uintptr_t retaddr = 0;
        intmax_t reqpages = n;

        // Iterate through each page directory entry in range.
        for(size_t i = pdeindex(start); i < pdeindex(end); i++) {
            pd_entry_t *pde = &dir->entries[i];
            pt_table_t *tbl = (pt_table_t*) getframeaddr(pde);

            if(testattribute(pde, PDE_PRESENT_BIT)) {
                // Iterate through and check each page table entry.
                for(uint32_t j = 0; j < PGE_PAGES_PER_TABLE; j++) {
                    pt_entry_t *pte = &tbl->entries[j];

                    // If it's present, reset. Not contiguous.
                    if(testattribute(pte, PTE_PRESENT_BIT)) {
                        addrcached = false;
                        reqpages = n;
                        continue;
                    } else if(!addrcached) {
                        addrcached = true;
                        retaddr = (i << 22) | (j << 12);
                    }

                    // If we have enough free pages, return.
                    if(--reqpages <= 0) return retaddr;
                }
            } else {
                // A page table contains 1024 page entries.
                reqpages -= PGE_PAGES_PER_TABLE;

                // Set the return address if this is the first free area.
                if(!addrcached) {
                    addrcached = true;
                    retaddr = (i << 22);
                }

                if(reqpages <= 0) return retaddr;
            }
        }

        return 0;
    }

    void mappage(pd_directory_t *dir, uintptr_t virt, uintptr_t phys) {
        pd_entry_t *pde = &dir->entries[pdeindex(virt)];
        pt_table_t *tbl;

        if(testattribute(pde, PDE_PRESENT_BIT)) {
            // PDE is present, get the table's address.
            tbl = (pt_table_t*) getframeaddr(pde);
        } else {
            // PDE not present, allocate a new page table.
            tbl = (pt_table_t*) kmalloc(sizeof(pt_table_t));
            if(tbl) memset(tbl, 0, sizeof(pt_table_t));
            else panic("kernel is out of memory!");

            // Map the new page table into the PDE.
            addattribute(pde, PDE_PRESENT_BIT | PDE_WRITABLE_BIT);
            setframe(pde, (uint32_t) tbl);
        }

        // Fetch the address of the page table entry and map it in.
        pt_entry_t *pge = &tbl->entries[pteindex(virt)];
        addattribute(pge, PTE_PRESENT_BIT | PTE_WRITABLE_BIT);
        setframe(pge, phys);
    }

    void *allocatevirt(pd_directory_t *dir, uintptr_t start, uintptr_t end, size_t n) {
        // We are free to allocate up to 0xC0000000, the kernel's virtual base.
        uintptr_t virt = findfirstfree(dir, start, end, n);
        uintptr_t phys = (uintptr_t) mem::allocatephys(n);
        uintptr_t vmax = virt + (n * PGE_INDIVIDUAL_PAGE_SIZE);

        if(virt == 0) panic("system has run out of virtual memory.");
        if(phys == 0) panic("system has run out of physical memory.");

        // Map our free virtual address space to some physical blocks of memory.
        for(uintptr_t virtaddr = virt; virtaddr < vmax; phys += 0x1000, virtaddr += 0x1000) mappage(dir, virtaddr, phys);
        return (void*) virt;
    }

    void freevirt(pd_directory_t *dir, uintptr_t base, size_t n) {
        if(n == 0) return;

        uintptr_t bmax = base + (n * PGE_INDIVIDUAL_PAGE_SIZE);
        uint32_t tableidx = pteindex(base);
        uint32_t cleared = 0;

        for(uint32_t i = pdeindex(base); i < pdeindex(bmax) + 1; i++) {
            // We assume that what we're freeing is actually allocated.
            pd_entry_t *pde = &dir->entries[i];
            pt_table_t *tbl = (pt_table_t*) getframeaddr(pde);

            // Keep looping until either the table is exhausted or we've cleared enough.
            while(cleared < n && tableidx < PGE_PAGES_PER_TABLE) {
                pt_entry_t *pte = &tbl->entries[tableidx++];
                delattribute(pte, PTE_PRESENT_BIT | PTE_WRITABLE_BIT);
                mem::freephys(getframeaddr(pte), 1);
                cleared++;
            }

            // At this point, either we've cleared enough blocks or there's another PDE we have to free.
            if(cleared == n) return;
            tableidx = 0;
        }
    }

    void initialisevirt(mb_info_t *mbd) {
        // Zero out the kernel page tables and the page directory.
        memset(kernelpt, 0, sizeof(pt_table_t) * PGE_TABLES_PER_DIRECTORY);
        memset(kpdvirt, 0, sizeof(pd_directory_t));
        currentdir = kpdvirt;

        // Link the kernel page tables into the page directory.
        for(uint32_t i = 0; i < PGE_TABLES_PER_DIRECTORY; i++) {
            addattribute(&kpdvirt->entries[i], PDE_PRESENT_BIT | PDE_WRITABLE_BIT);
            setframe(&kpdvirt->entries[i], (uint32_t) &kernelpt[i] - 0xC0000000);
        }

        // Identity map eight megabytes of physical memory, the kernel's page directory is somewhere in here.
        for(uintptr_t idmap = 0x0; idmap < 0x800000; idmap += 0x1000) mappage(kpdvirt, idmap, idmap);

        // Map sixteen megabytes to the kernel's virtual base of 0xC0000000.
        for(uintptr_t kernelphys = 0x100000; kernelphys < 0x1000000; kernelphys += 0x1000) mappage(kpdvirt, kernelphys + 0xC0000000, kernelphys);

        // Calculate the size of the framebuffer and map the required amount of pages to 0xFC000000.
        uintptr_t bufferend = mbd->framebufferaddr + (mbd->framebufferpitch * mbd->framebufferheight);
        for(uintptr_t framephys = mbd->framebufferaddr, framevirt = 0xFC000000; framephys < bufferend; framephys += 0x1000, framevirt += 0x1000) mappage(kpdvirt, framevirt, framephys);
        mbd->framebufferaddr = 0xFC000000;

        // Check whether there are any GRUB modules loaded.
        if(checkbit(mbd->flags, 3) && mbd->modcount > 0) {
            mb_modlist_t *mods = (mb_modlist_t*) mbd->modaddr;
            uintptr_t modulevirt = 0xD0000000;

            // If so, map them into the kernel heap.
            for(uint32_t i = 0; i < mbd->modcount; i++) {
                uintptr_t virtstart = (i == 0) ? 0xD0000000 : modulevirt - (mods[i].modend - mods[i].modstart);
                for(uintptr_t modulephys = mods[i].modstart; modulephys < mods[i].modend; modulephys += 0x1000, modulevirt += 0x1000) mappage(kpdvirt, modulevirt, modulephys);
                mods[i].modstart = virtstart;
                mods[i].modend = modulevirt;
            }
        }

        // Register interrupt handlers and switch the active page directory.
        idt::registerhandler(14, &pfhandler);
        idt::registerhandler(6, &udhandler);

        // Load the physical address of the page directory into CR3.
        loadcr3((uint32_t) kpdphys);
    }
}
