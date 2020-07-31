#include <interrupts.hpp>
#include <mem/virt.hpp>
#include <mem/phys.hpp>
#include <mem/libc.hpp>
#include <errors.hpp>
#include <stdint.h>

extern "C" {
    // Control register load/store functions.
    void loadcr2(uintptr_t address);
    void loadcr3(uintptr_t address);
    uint64_t readcr2(void);
    uint64_t readcr3(void);
}

// The kernel's PML4, found in cabs.asm.
extern "C" mem::pml4_table_t *kernelpml4;

namespace mem {
    pml4_table_t *currentpml4;

    static inline void addattribute(uint64_t *entry, uint64_t attribute) {
        *entry |= attribute;
    }

    static inline void delattribute(uint64_t *entry, uint64_t attribute) {
        *entry &= ~attribute;
    }

    static inline bool testattribute(uint64_t *entry, uint64_t attribute) {
        return *entry & attribute;
    }

    static inline void setframeaddr(uint64_t *entry, uint64_t physaddr) {
        *entry = (*entry & ~PGE_FRAME_BITS) | physaddr;
    }

    static inline uintptr_t getframeaddr(uint64_t *entry) {
        return *entry & PGE_FRAME_BITS;
    }

    static inline void setcacheaddr(bool *cacheset, uintptr_t *addr, size_t pml4, size_t pdpt, size_t pd, size_t pt) {
        // Set the address based on cacheset.
        if(*cacheset == false) {
            *addr = pml4 << 39 | pdpt << 30 | pd << 21 | pt << 12;
            if(*addr & PGE_SIGNEXTENSION_BIT) *addr |= 0xFFFF000000000000;
            *cacheset = true;
        }
    }

    pml4_table_t *getkernelpml4(void) {
        return kernelpml4;
    }

    uintptr_t findfirstfree(pml4_table_t *pml4, uintptr_t start, uintptr_t end, size_t n) {
        uintptr_t retaddr = 0;
        bool addrcached = false;
        intmax_t reqpages = n;
        if(n == 0) n = 1;

        // Set table indexes to their initial values.
        size_t j = pdpeindex(start);
        size_t k = pdeindex(start);
        size_t l = pteindex(start);

        // Iterate through each PML4E entry in range.
        for(size_t i = pml4index(start); i < pml4index(end) + 1; i++) {
            pml4_entry_t *pml4e = &pml4->entries[i];

            if(testattribute(pml4e, PGE_PRESENT_BIT)) {
                // Because the PML4E was present, iterate through it's PDPT.
                pdp_table_t *pdpt = (pdp_table_t*) getframeaddr(pml4e);
                for(; j < PGE_ENTRIES_PER_STRUCTURE; j++) {
                    pdp_entry_t *pdpe = &pdpt->entries[j];

                    if(testattribute(pdpe, PGE_PRESENT_BIT)) {
                        // If the PDPE is a 1GB entry, reset parameters and continue.
                        if(testattribute(pdpe, PGE_HUGEPAGE_BIT)) {
                            addrcached = false;
                            reqpages = n;
                            continue;
                        }

                        // Because the PDPE was present, we must search through the PDTs and PTs.
                        pd_directory_t *pdt = (pd_directory_t*) getframeaddr(pdpe);
                        for(; k < PGE_ENTRIES_PER_STRUCTURE; k++) {
                            pd_entry_t *pde = &pdt->entries[k];

                            if(testattribute(pde, PGE_PRESENT_BIT)) {
                                // If the PDE is a 2MB entry, reset parameters and continue.
                                if(testattribute(pde, PGE_HUGEPAGE_BIT)) {
                                    addrcached = false;
                                    reqpages = n;
                                    continue;
                                }

                                // Because the PDE was present, we must search through the page tables.
                                pt_table_t *pt = (pt_table_t*) getframeaddr(pde);
                                for(; l < PGE_ENTRIES_PER_STRUCTURE; l++) {
                                    pt_entry_t *pte = &pt->entries[l];

                                    if(testattribute(pte, PGE_PRESENT_BIT)) {
                                        // We can go no further. Reset parameters and continue.
                                        addrcached = false;
                                        reqpages = n;
                                        continue;
                                    } else {
                                        setcacheaddr(&addrcached, &retaddr, i, j, k, l); // Free page!
                                        if(--reqpages <= 0) return retaddr;
                                    }
                                }

                                // PTE reset.
                                l = 0;
                            } else {
                                setcacheaddr(&addrcached, &retaddr, i, j, k, 0); // PDE not present! (512 free pages).
                                if((reqpages -= PGE_PDE_ADDRSPACE / 0x1000) <= 0) return retaddr;
                            }
                        }

                        // PDE reset.
                        k = 0;
                    } else {
                        setcacheaddr(&addrcached, &retaddr, i, j, 0, 0); // PDPE not present! (256k free pages).
                        if((reqpages -= PGE_PDPE_ADDRSPACE / 0x1000) <= 0) return retaddr;
                    }
                }

                // PDPE reset.
                j = 0;
            } else {
                setcacheaddr(&addrcached, &retaddr, i, 0, 0, 0); // PML4E not present! (~134M free pages).
                if((reqpages -= PGE_PML4E_ADDRSPACE / 0x1000) <= 0) return retaddr;
            }
        }

        return 0; // Didn't find anything.
    }

    void mappage(pml4_table_t *pml4, mem_pagetype_t type, uintptr_t virt, uintptr_t phys) {
        pml4_entry_t *pml4e = &pml4->entries[pml4index(virt)];

        if(type == PGE_HUGE1GB_PAGE) {
            pdp_table_t *pdpt = (pdp_table_t*) getframeaddr(pml4e);
            pdp_entry_t *pdpe = &pdpt->entries[pdpeindex(virt)];

            if(!testattribute(pml4e, PGE_PRESENT_BIT)) {
                // If the PML4E isn't present, allocate a new PDPT.
                pdpt = (pdp_table_t*) mem::allocatephys(1);
                pdpe = &pdpt->entries[pdpeindex(virt)];
                memset(pdpt, 0, sizeof(pdp_table_t));

                // Point the PML4E to the new PDPT and mark as present.
                addattribute(pml4e, PGE_PRESENT_BIT | PGE_WRITABLE_BIT);
                setframeaddr(pml4e, (uint64_t) pdpt);
            }

            // Map the address and set the present, read/write and huge page flags.
            addattribute(pdpe, PGE_PRESENT_BIT | PGE_WRITABLE_BIT | PGE_HUGEPAGE_BIT);
            setframeaddr(pdpe, phys & 0x000FFFFFC0000000);
        }

        else if(type == PGE_HUGE2MB_PAGE) {
            pdp_table_t *pdpt = (pdp_table_t*) getframeaddr(pml4e);
            pdp_entry_t *pdpe = &pdpt->entries[pdpeindex(virt)];
            pd_directory_t *pdt = (pd_directory_t*) getframeaddr(pdpe);
            pd_entry_t *pde = &pdt->entries[pdeindex(virt)];

            if(!testattribute(pml4e, PGE_PRESENT_BIT)) {
                // PML4E isn't present, therefore neither is the PDPT or PDT.
                pdpt = (pdp_table_t*) mem::allocatephys(1);
                pdpe = &pdpt->entries[pdpeindex(virt)];
                memset(pdpt, 0, sizeof(pdp_table_t));

                // Point the PML4E to the new PDPT and mark as present.
                addattribute(pml4e, PGE_PRESENT_BIT | PGE_WRITABLE_BIT);
                setframeaddr(pml4e, (uint64_t) pdpt);
            }

            if(!testattribute(pdpe, PGE_PRESENT_BIT)) {
                // PDPT isn't present, therefore neither is the PDT.
                pdt = (pd_directory_t*) mem::allocatephys(1);
                pde = &pdt->entries[pdeindex(virt)];
                memset(pdt, 0, sizeof(pd_directory_t));

                // Point the PDPE to the new PDT and mark as present.
                addattribute(pdpe, PGE_PRESENT_BIT | PGE_WRITABLE_BIT);
                setframeaddr(pdpe, (uint64_t) pdt);
            }

            // Map the address and set the present, read/write and huge page flags.
            addattribute(pde, PGE_PRESENT_BIT | PGE_WRITABLE_BIT | PGE_HUGEPAGE_BIT);
            setframeaddr(pde, phys & 0x000FFFFFFFE00000);
        }

        else if(type == PGE_REGULAR_PAGE) {
            pdp_table_t *pdpt = (pdp_table_t*) getframeaddr(pml4e);
            pdp_entry_t *pdpe = &pdpt->entries[pdpeindex(virt)];
            pd_directory_t *pdt = (pd_directory_t*) getframeaddr(pdpe);
            pd_entry_t *pde = &pdt->entries[pdeindex(virt)];
            pt_table_t *ptt = (pt_table_t*) getframeaddr(pde);
            pt_entry_t *pte = &ptt->entries[pteindex(virt)];

            if(!testattribute(pml4e, PGE_PRESENT_BIT)) {
                // PML4E isn't present, therefore neither is the PDPT or PDT.
                pdpt = (pdp_table_t*) mem::allocatephys(1);
                pdpe = &pdpt->entries[pdpeindex(virt)];
                memset(pdpt, 0, sizeof(pdp_table_t));

                // Point the PML4E to the new PDPT and mark as present.
                addattribute(pml4e, PGE_PRESENT_BIT | PGE_WRITABLE_BIT);
                setframeaddr(pml4e, (uint64_t) pdpt);
            }

            if(!testattribute(pdpe, PGE_PRESENT_BIT)) {
                // PDPT isn't present, therefore neither is the PDT.
                pdt = (pd_directory_t*) mem::allocatephys(1);
                pde = &pdt->entries[pdeindex(virt)];
                memset(pdt, 0, sizeof(pd_directory_t));

                // Point the PDPE to the new PDT and mark as present.
                addattribute(pdpe, PGE_PRESENT_BIT | PGE_WRITABLE_BIT);
                setframeaddr(pdpe, (uint64_t) pdt);
            }

            if(!testattribute(pde, PGE_PRESENT_BIT)) {
                // PDT isn't present, therefore neither is the PT.
                ptt = (pt_table_t*) mem::allocatephys(1);
                pte = &ptt->entries[pteindex(virt)];
                memset(ptt, 0, sizeof(pt_table_t));

                // Point the PDE to the new PT and mark as present.
                addattribute(pde, PGE_PRESENT_BIT | PGE_WRITABLE_BIT);
                setframeaddr(pde, (uint64_t) ptt);
            }

            // Map the address and set the present, read/write and huge page flags.
            addattribute(pte, PGE_PRESENT_BIT | PGE_WRITABLE_BIT);
            setframeaddr(pte, phys);
        }
    }

    void *allocatevirt(pml4_table_t *pml4, uintptr_t start, uintptr_t end, size_t n) {
        uintptr_t virt = findfirstfree(pml4, start, end, n);
        uintptr_t phys = (uintptr_t) mem::allocatephys(n);
        uintptr_t vmax = virt + (n * PGE_PTE_ADDRSPACE);

        // Return NULL if there is no free virtual address space left.
        if(virt == 0) return nullptr;

        // Map our free virtual address space to some physical blocks of memory.
        for(uintptr_t virtaddr = virt; virtaddr < vmax; phys += 0x1000, virtaddr += 0x1000) mappage(pml4, PGE_REGULAR_PAGE, virtaddr, phys);
        return (void*) virt;
    }

    void freevirt(pml4_table_t *pml4, uintptr_t base, size_t n) {
        uintptr_t addrspace = n * PGE_PTE_ADDRSPACE;
        uintptr_t bmax = base + addrspace;
        size_t cleared = 0;

        // Set table indexes to their initial values.
        size_t pdpeidx = pdpeindex(base);
        size_t pdeidx = pdeindex(base);
        size_t pteidx = pteindex(base);

        // Iterate through every PML4E in range.
        for(size_t i = pml4index(base); i < pml4index(bmax) + 1; i++) {
            pml4_entry_t *pml4e = &pml4->entries[i];
            pdp_table_t *pdpt = (pdp_table_t*) getframeaddr(pml4e);
            if(addrspace > PGE_PML4E_ADDRSPACE) delattribute(pml4e, PGE_PRESENT_BIT | PGE_WRITABLE_BIT);

            // Iterate through every PDPE inside the PDPT.
            while(pdpeidx < PGE_ENTRIES_PER_STRUCTURE) {
                pdp_entry_t *pdpe = &pdpt->entries[pdpeidx++];
                pd_directory_t *pdt = (pd_directory_t*) getframeaddr(pdpe);
                if(addrspace > PGE_PDPE_ADDRSPACE) delattribute(pdpe, PGE_PRESENT_BIT | PGE_WRITABLE_BIT);

                // If it's a huge PDPE, free it and skip any PDTs and PTs.
                if(testattribute(pdpe, PGE_HUGEPAGE_BIT)) {
                    mem::freephys(getframeaddr(pdpe), PGE_PDPE_ADDRSPACE / PMMGR_BLOCK_SIZE);
                    cleared += PGE_PDPE_ADDRSPACE / PMMGR_BLOCK_SIZE;
                    addrspace -= PGE_PDPE_ADDRSPACE;
                    continue;
                }

                // Iterate through every PDE inside the PDT.
                while(pdeidx < PGE_ENTRIES_PER_STRUCTURE) {
                    pd_entry_t *pde = &pdt->entries[pdeidx++];
                    pt_table_t *ptt = (pt_table_t*) getframeaddr(pde);
                    if(addrspace > PGE_PDE_ADDRSPACE) delattribute(pde, PGE_PRESENT_BIT | PGE_WRITABLE_BIT);

                    // If it's a huge PDE, free it and skip any PTs.
                    if(testattribute(pde, PGE_HUGEPAGE_BIT)) {
                        mem::freephys(getframeaddr(pde), PGE_PDE_ADDRSPACE / PMMGR_BLOCK_SIZE);
                        cleared += PGE_PDE_ADDRSPACE / PMMGR_BLOCK_SIZE;
                        addrspace -= PGE_PDE_ADDRSPACE;
                        continue;
                    }

                    // Iterate through every page table entry.
                    while(pteidx < PGE_ENTRIES_PER_STRUCTURE) {
                        pt_entry_t *pte = &ptt->entries[pteidx++];
                        mem::freephys(getframeaddr(pte), 1);
                        delattribute(pte, PGE_PRESENT_BIT | PGE_WRITABLE_BIT);
                        addrspace -= PGE_PTE_ADDRSPACE;
                        cleared++;
                    }

                    if(cleared >= n) return;
                    pteidx = 0;
                }

                if(cleared >= n) return;
                pdeidx = 0;
            }

            if(cleared >= n) return;
            pdpeidx = 0;
        }
    }

    void pfhandler(idt::regs64_t *regs) {
        // Read the required control registers into C variables.
        uint64_t cr2 = readcr2();
        uint64_t cr3 = readcr3();
        const char *reason;

        switch(regs->err) {
            case 0x0: reason = "supervisor process tried to read a non-present page entry."; break;
            case 0x1: reason = "supervisory process tried to read a page and caused a protection fault."; break;
            case 0x2: reason = "supervisory process tried to write to a non-present page entry."; break;
            case 0x3: reason = "supervisory process tried to write a page and caused a protection fault."; break;
            case 0x4: reason = "user process tried to read a non-present page entry."; break;
            case 0x5: reason = "user process tried to read a page and caused a protection fault."; break;
            case 0x6: reason = "user process tried to write to a non-present page entry."; break;
            case 0x7: reason = "user process tried to write a page and caused a protection fault."; break;
            default: reason = "unknown."; break;
        }

        ctxpanic(regs, "Page fault exception, %s\n"
                       "[kpanic] cr2 (page address): %p\n"
                       "[kpanic] cr3 (PML4 address): %p",
                       reason, cr2, cr3);
    }

    void initialisevirt(void) {
        // Identity map the first two megabytes except for 0x0.
        for(uintptr_t p = 0x1000; p < 0x200000; p += 0x1000) {
            mappage(kernelpml4, PGE_REGULAR_PAGE, p, p);
        }

        // Map the kernel into the higher half of the VAS.
        for(uintptr_t p = 0x100000, v = p + PGE_KERNEL_VBASE; v < (uintptr_t) &kernelend; p += 0x1000, v += 0x1000) {
            mappage(kernelpml4, PGE_REGULAR_PAGE, v, p);
        }

        // Map the framebuffer into the kernel's address space.
        uintptr_t pend = mboot::info.fbinfo->common.framebuffer + (mboot::info.fbinfo->common.height * mboot::info.fbinfo->common.width * (mboot::info.fbinfo->common.bpp / 8));
        for(uintptr_t p = mboot::info.fbinfo->common.framebuffer, v = 0xFFFFFFFFC0000000; p < pend; p += 0x1000, v += 0x1000) {
            mappage(kernelpml4, PGE_REGULAR_PAGE, v, p);
        }

        // Register mem::pfhandler() as the page fault handler.
        idt::registerhandler(14, &pfhandler);

        // Change the framebuffer, mark bootstrap structures as free and reload CR3.
        mboot::info.fbinfo->common.framebuffer = 0xFFFFFFFFC0000000;
        markphysfree(pge64sel[0], pge64sel[2]);
        loadcr3((uintptr_t) kernelpml4);
    }
}
