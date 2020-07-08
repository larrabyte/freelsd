#include <mem/virt.hpp>
#include <mem/phys.hpp>
#include <mem/libc.hpp>
#include <serial.hpp>
#include <stdint.h>

namespace mem {
    pml4_table_t *kernelpml4;
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

    uintptr_t findfirstfree(pml4_table_t *pml4, uintptr_t start, uintptr_t end, size_t n) {
        uintptr_t retaddr = 0;
        bool addrcached = false;
        intmax_t reqpages = n;
        if(n == 0) n = 1;

        // Iterate through each PML4E entry in range.
        for(size_t i = pml4index(start); i < pml4index(end); i++) {
            pml4_entry_t *pml4e = &pml4->entries[i];

            if(testattribute(pml4e, PGE_PRESENT_BIT)) {
                // Because the PML4E was present, iterate through it's PDPT.
                pdp_table_t *pdpt = (pdp_table_t*) getframeaddr(pml4e);
                for(size_t j = 0; j < PGE_ENTRIES_PER_STRUCTURE; j++) {
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
                        for(size_t k = 0; k < PGE_ENTRIES_PER_STRUCTURE; k++) {
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
                                for(size_t l = 0; l < PGE_ENTRIES_PER_STRUCTURE; l++) {
                                    pt_entry_t *pte = &pt->entries[l];

                                    if(testattribute(pte, PGE_PRESENT_BIT)) {
                                        // We can go no further. Reset parameters and continue.
                                        addrcached = false;
                                        reqpages = n;
                                        continue;
                                    } else {
                                        // The page table entry isn't present, we have a free page!
                                        setcacheaddr(&addrcached, &retaddr, i, j, k, l);
                                        if(--reqpages <= 0) return retaddr;
                                    }
                                }
                            } else {
                                // The PDE isn't present, we have a contiguous chunk of 512 free pages.
                                setcacheaddr(&addrcached, &retaddr, i, j, k, 0);
                                reqpages -= PGE_PDE_ADDRSPACE / 0x1000;
                                if(reqpages <= 0) return retaddr;
                            }
                        }
                    } else {
                        // The PDPE isn't present, we have a contiguous chunk of ~256k free pages.
                        setcacheaddr(&addrcached, &retaddr, i, j, 0, 0);
                        reqpages -= PGE_PDPE_ADDRSPACE / 0x1000;
                        if(reqpages <= 0) return retaddr;
                    }
                }
            } else {
                // The PML4E is not present, we have a contiguous chunk of ~134M free pages.
                setcacheaddr(&addrcached, &retaddr, i, 0, 0, 0);
                reqpages -= PGE_PML4E_ADDRSPACE / 0x1000;
                if(reqpages <= 0) return retaddr;
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

        if(virt == 0) serial::printf("[virtmm] system has run out of virtual memory.\n");
        if(phys == 0) serial::printf("[virtmm] system has run out of physical memory.\n");

        // Map our free virtual address space to some physical blocks of memory.
        for(uintptr_t virtaddr = virt; virtaddr < vmax; phys += 0x1000, virtaddr += 0x1000) mappage(pml4, PGE_REGULAR_PAGE, virtaddr, phys);
        return (void*) virt;
    }

    /* void freevirt(pml4_table_t *pml4, uintptr_t base, size_t n) {
        uintptr_t addrspace = n * PGE_PTE_ADDRSPACE;
        uintptr_t bmax = base + addrspace;
        size_t curidx = pdpeindex(base);
        size_t cleared = 0;

        for(size_t i = pml4index(base); i < pml4index(bmax) + 1; i++) {
            pml4_entry_t *pml4e = &pml4->entries[i];
            pdp_table_t *pdpt = (pdp_table_t*) getframeaddr(pml4e);
            if(addrspace > PGE_PML4E_ADDRSPACE) delattribute(pml4e, PGE_PRESENT_BIT | PGE_WRITABLE_BIT);

            for(size_t j = pdpeindex(base); j < PGE_ENTRIES_PER_STRUCTURE; j++) {
                pdp_entry_t *pdpe = &pdpt->entries[j];
                pd_directory_t *pdt = (pd_directory_t*) getframeaddr(pdpe);
                if(addrspace > PGE_PDPE_ADDRSPACE) delattribute(pdpe, PGE_PRESENT_BIT | PGE_WRITABLE_BIT);

                if(testattribute(pdpe, PGE_HUGEPAGE_BIT)) {
                    mem::freephys(getframeaddr(pdpe), PGE_PDPE_ADDRSPACE / PMMGR_BLOCK_SIZE);
                    addrspace -= PGE_PDPE_ADDRSPACE;
                    continue;
                }

                for(size_t k = pdeindex(base); k < PGE_ENTRIES_PER_STRUCTURE; k++) {
                    pd_entry_t *pde = &pdt->entries[k];
                    pt_table_t *ptt = (pt_table_t*) getframeaddr(pde);
                    if(addrspace > PGE_PDE_ADDRSPACE) delattribute(pde, PGE_PRESENT_BIT | PGE_WRITABLE_BIT);

                    if(testattribute(pde, PGE_HUGEPAGE_BIT)) {
                        mem::freephys(getframeaddr(pde), PGE_PDE_ADDRSPACE / PMMGR_BLOCK_SIZE);
                        addrspace -= PGE_PDE_ADDRSPACE;
                        continue;
                    }

                    size_t l = pteindex(base);

                    while(l < PGE_ENTRIES_PER_STRUCTURE) {
                        pt_entry_t *pte = &ptt->entries[l];
                        mem::freephys(getframeaddr(pte), 1);
                        delattribute(pte, PGE_PRESENT_BIT | PGE_WRITABLE_BIT);
                        addrspace -= PGE_PTE_ADDRSPACE;
                    }

                    if(addrspace == 0) return;
                    l = 0;
                }
            }
        }
    } */

    void initialisevirt(void) {
        // Set the address of the kernel's PML4 to the bootstrap PML4.
        kernelpml4 = currentpml4 = 0x0000000000000000;

        // Map the framebuffer into the address space.
        uintptr_t fbpend = mboot::info.fbinfo->common.framebuffer + (mboot::info.fbinfo->common.height * mboot::info.fbinfo->common.width);
        for(uintptr_t fbp = mboot::info.fbinfo->common.framebuffer, fbv = 0x00007FFFFEFFF000; fbp < fbpend; fbp += 0x1000, fbv += 0x1000) {
            mappage(kernelpml4, PGE_REGULAR_PAGE, fbv, fbp);
        }

        // Set the framebuffer to the new virtual address.
        mboot::info.fbinfo->common.framebuffer = 0x00007FFFFEFFF000;
    }
}
