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
extern "C" mem::pge_structure_t *kernelpml4;

namespace mem {
    pge_structure_t *currentpml4;

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

    static inline pge_structure_t *getframeaddr(uint64_t *entry) {
        return (pge_structure_t*) (*entry & PGE_FRAME_BITS);
    }

    pge_structure_t *getkernelpml4(void) {
        return kernelpml4;
    }

    uintptr_t findfirstfree(pge_structure_t *pml4, uintptr_t start, uintptr_t end, size_t n) {
        uintptr_t retaddr = 0x0;
        bool addrcached = false;
        intmax_t reqpages = n;

        // Set table indexes to their initial values.
        size_t j = pdpeindex(start);
        size_t k = pdeindex(start);
        size_t l = pteindex(start);

        // Iterate through each PML4 entry in range.
        for(size_t i = pml4index(start); i < pml4index(end) + 1; i++) {
            pge_entry_t *pml4e = &pml4->entries[i];

            if(testattribute(pml4e, PGE_PRESENT_BIT)) {
                // PML4E is present, iterate through it's PDPT.
                pge_structure_t *pdpt = getframeaddr(pml4e);
                while(j < PGE_ENTRIES_PER_STRUCTURE) {
                    pge_entry_t *pdpe = &pdpt->entries[j++];

                    if(testattribute(pdpe, PGE_PRESENT_BIT)) {
                        // If PDPE is present & huge, reset parameters.
                        if(testattribute(pdpe, PGE_HUGEPAGE_BIT)) {
                            addrcached = false;
                            reqpages = n;
                            continue;
                        }

                        // PDPE wasn't huge, iterate through it's PDT.
                        pge_structure_t *pdt = getframeaddr(pdpe);
                        while (k < PGE_ENTRIES_PER_STRUCTURE) {
                            pge_entry_t *pde = &pdt->entries[k++];

                            if(testattribute(pde, PGE_PRESENT_BIT)) {
                                // If PDE is present & huge, reset parameters.
                                if(testattribute(pde, PGE_HUGEPAGE_BIT)) {
                                    addrcached = false;
                                    reqpages = n;
                                    continue;
                                }

                                // PDE wasn't huge, iterate through it's page table.
                                pge_structure_t *pt = getframeaddr(pde);
                                while(l < PGE_ENTRIES_PER_STRUCTURE) {
                                    pge_entry_t *pte = &pt->entries[l++];

                                    // If PTE is present, we can't go any further. Reset parameters.
                                    if(testattribute(pte, PGE_PRESENT_BIT)) {
                                        addrcached = false;
                                        reqpages = n;
                                        continue;
                                    } else {
                                        pgecacheaddr(addrcached, retaddr, i, j, k, l); // Free page!
                                        if(--reqpages <= 0) return retaddr;
                                    }
                                }

                                l = 0; // Reset PT index upon next PDE iteration.
                            } else {
                                pgecacheaddr(addrcached, retaddr, i, j, k, 0); // Free PDE: 512 free pages.
                                if((reqpages -= PGE_PDE_ADDRSPACE / PGE_PTE_ADDRSPACE) <= 0) return retaddr;
                            }
                        }

                        k = 0; // Reset PDT index upon next PDPE iteration.
                    } else {
                        pgecacheaddr(addrcached, retaddr, i, j, 0, 0); // Free PDPE: 256k free pages.
                        if((reqpages -= PGE_PDPE_ADDRSPACE / PGE_PTE_ADDRSPACE) <= 0) return retaddr;
                    }
                }

                j = 0; // Reset PDPT index upon next PML4E iteration.
            } else {
                pgecacheaddr(addrcached, retaddr, i, 0, 0, 0); // Free PML4E: ~134M free pages.
                if((reqpages -= PGE_PML4E_ADDRSPACE / PGE_PTE_ADDRSPACE) <= 0) return retaddr;
            }
        }

        // Didn't find anything.
        return 0;
    }

    void mappage(pge_structure_t *pml4, mem_pagetype_t type, uintptr_t virt, uintptr_t phys, bool allocated) {
        pge_entry_t *pml4e = &pml4->entries[pml4index(virt)];

        // If the PML4E isn't present, allocate a new PDPT and mark the PML4E as present.
        pge_structure_t *pdpt = getframeaddr(pml4e);
        if(!testattribute(pml4e, PGE_PRESENT_BIT)) { pgeallocstruct(pdpt, pml4e); }
        pge_entry_t *pdpe = &pdpt->entries[pdpeindex(virt)];

        if(type == PGE_HUGE1GB_PAGE) {
            // Set the hugepage bit if required and return, job finished.
            addattribute(pdpe, PGE_PRESENT_BIT | PGE_WRITABLE_BIT | PGE_HUGEPAGE_BIT);
            if(allocated) addattribute(pdpe, PGE_ALLOCATED_BIT);
            setframeaddr(pdpe, phys & 0x000FFFFFC0000000);
            return;
        }

        // If the PDPE isn't present, allocate a new PDT and mark the PDPE as present.
        pge_structure_t *pdt = getframeaddr(pdpe);
        if(!testattribute(pdpe, PGE_PRESENT_BIT)) { pgeallocstruct(pdt, pdpe); }
        pge_entry_t *pde = &pdt->entries[pdeindex(virt)];

        if(type == PGE_HUGE2MB_PAGE) {
            // Set the hugepage bit if required and return, job finished.
            addattribute(pde, PGE_PRESENT_BIT | PGE_WRITABLE_BIT | PGE_HUGEPAGE_BIT);
            if(allocated) addattribute(pde, PGE_ALLOCATED_BIT);
            setframeaddr(pde, phys & 0x000FFFFFFFE00000);
            return;
        }

        // If the PDE isn't present, allocate a new page table and mark the PDE as present.
        pge_structure_t *pt = getframeaddr(pde);
        if(!testattribute(pde, PGE_PRESENT_BIT)) { pgeallocstruct(pt, pde); }
        pge_entry_t *pte = &pt->entries[pteindex(virt)];

        if(type == PGE_REGULAR_PAGE) {
            addattribute(pte, PGE_PRESENT_BIT | PGE_WRITABLE_BIT);
            if(allocated) addattribute(pte, PGE_ALLOCATED_BIT);
            setframeaddr(pte, phys);
            return;
        }
    }

    void unmappage(pge_structure_t *pml4, uintptr_t base, size_t n) {
        uintptr_t bmax = base + (n * PGE_PTE_ADDRSPACE);
        size_t cleared = 0;

        // Set table indexes to their initial values.
        size_t j = pdpeindex(base);
        size_t k = pdeindex(base);
        size_t l = pteindex(base);

        for(size_t i = pml4index(base); i < pml4index(bmax) + 1; i++) {
            pge_entry_t *pml4e = &pml4->entries[i];
            pge_structure_t *pdpt = getframeaddr(pml4e);

            while(j < PGE_ENTRIES_PER_STRUCTURE) {
                pge_entry_t *pdpe = &pdpt->entries[j++];
                pge_structure_t *pdt = getframeaddr(pdpe);

                // If it's a huge PDPE: delete all attributes and free if necessary.
                if(testattribute(pdpe, PGE_HUGEPAGE_BIT)) {
                    if(testattribute(pdpe, PGE_ALLOCATED_BIT)) mem::freephys((uintptr_t) getframeaddr(pdpe), PGE_PDPE_ADDRSPACE / PMMGR_BLOCK_SIZE);
                    delattribute(pdpe, PGE_PRESENT_BIT | PGE_WRITABLE_BIT | PGE_HUGEPAGE_BIT);
                    cleared += PGE_PDPE_ADDRSPACE / PGE_PTE_ADDRSPACE;
                    continue;
                }

                while(k < PGE_ENTRIES_PER_STRUCTURE) {
                    pge_entry_t *pde = &pdt->entries[k++];
                    pge_structure_t *pt = getframeaddr(pde);

                    // If it's a huge PDE: delete all attributes and free if necessary.
                    if(testattribute(pde, PGE_HUGEPAGE_BIT)) {
                        if(testattribute(pde, PGE_ALLOCATED_BIT)) mem::freephys((uintptr_t) getframeaddr(pde), PGE_PDE_ADDRSPACE / PMMGR_BLOCK_SIZE);
                        delattribute(pde, PGE_PRESENT_BIT | PGE_WRITABLE_BIT | PGE_HUGEPAGE_BIT);
                        cleared += PGE_PDE_ADDRSPACE / PGE_PTE_ADDRSPACE;
                        continue;
                    }

                    while(l < PGE_ENTRIES_PER_STRUCTURE) {
                        pge_entry_t *pte = &pt->entries[l++];
                        if(testattribute(pde, PGE_ALLOCATED_BIT)) mem::freephys((uintptr_t) getframeaddr(pte), 1);
                        delattribute(pte, PGE_PRESENT_BIT | PGE_WRITABLE_BIT);
                        cleared++;
                    }

                    if(cleared >= n) return;
                    l = 0;
                }
    
                if(cleared >= n) return;
                k = 0;
            }

            if(cleared >= n) return;
            j = 0;
        }
    }

    void *allocatevirt(pge_structure_t *pml4, uintptr_t start, uintptr_t end, size_t n) {
        uintptr_t virt = findfirstfree(pml4, start, end, n);
        uintptr_t phys = (uintptr_t) mem::allocatephys(n);
        uintptr_t vmax = virt + (n * PGE_PTE_ADDRSPACE);

        // Return nullptr if there isn't any free virtual address space left.
        if(virt == 0) return nullptr;

        // Map the free virtual address space to some physical blocks of memory.
        for(uintptr_t v = virt; v < vmax; v += 0x1000, phys += 0x1000) mappage(pml4, PGE_REGULAR_PAGE, v, phys, true);
        return (void*) virt;
    }

    void freevirt(pge_structure_t *pml4, uintptr_t base, size_t n) {
        unmappage(pml4, base, n);
    }

    void pfhandler(idt::regs64_t *regs) {
        // Read the required control registers into C variables.
        uint64_t cr2 = readcr2();
        uint64_t cr3 = readcr3();
        const char *reason;

        switch(regs->err) {
            case 00: reason = "supervisor process tried to read a non-present page entry."; break;
            case 01: reason = "supervisory process tried to read a page and caused a protection fault."; break;
            case 02: reason = "supervisory process tried to write to a non-present page entry."; break;
            case 03: reason = "supervisory process tried to write a page and caused a protection fault."; break;
            case 04: reason = "user process tried to read a non-present page entry."; break;
            case 05: reason = "user process tried to read a page and caused a protection fault."; break;
            case 06: reason = "user process tried to write to a non-present page entry."; break;
            case 07: reason = "user process tried to write a page and caused a protection fault."; break;
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
            mappage(kernelpml4, PGE_REGULAR_PAGE, p, p, false);
        }

        // Map the kernel into the higher half of the VAS.
        for(uintptr_t p = 0x100000, v = p + PGE_KERNEL_VBASE; v < (uintptr_t) &kernelend; p += 0x1000, v += 0x1000) {
            mappage(kernelpml4, PGE_REGULAR_PAGE, v, p, false);
        }

        // Map the framebuffer into the kernel's address space.
        uintptr_t pend = mboot::info.fbinfo->common.framebuffer + (mboot::info.fbinfo->common.height * mboot::info.fbinfo->common.width * (mboot::info.fbinfo->common.bpp / 8));
        for(uintptr_t p = mboot::info.fbinfo->common.framebuffer, v = 0xFFFFFFFFC0000000; p < pend; p += 0x1000, v += 0x1000) {
            mappage(kernelpml4, PGE_REGULAR_PAGE, v, p, false);
        }

        // Register mem::pfhandler() as the page fault handler.
        idt::registerhandler(14, &pfhandler);

        // Change the framebuffer, mark bootstrap structures as free and reload CR3.
        mboot::info.fbinfo->common.framebuffer = 0xFFFFFFFFC0000000;
        markphysfree(pge64sel[0], pge64sel[2]);
        loadcr3((uintptr_t) kernelpml4);
    }
}
