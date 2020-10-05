#include <interrupts.hpp>
#include <multiboot.hpp>
#include <mem/virt.hpp>
#include <mem/phys.hpp>
#include <mem/libc.hpp>
#include <errors.hpp>

// Control register read/write functions.
extern "C" void writecr2(uintptr_t address);
extern "C" void writecr3(uintptr_t address);
extern "C" uint64_t readcr2(void);
extern "C" uint64_t readcr3(void);

// External kernel-related data.
extern "C" mem::pge_structure_t *kernelpml4;
extern "C" mem::bootdata_t pge64sel;
extern "C" void *kernelend;

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

    static inline void cacheaddress(bool *conditional, uintptr_t *cache, size_t pml4e, size_t pdpe, size_t pde, size_t pte) {
        if(!*conditional) {
            uintptr_t address = pml4e << 39 | pdpe << 30 | pde << 21 | pte << 12;
            if(address & PGE_SIGNEXTENSION_BIT) address |= 0xFFFF000000000000;
            *conditional = true;
            *cache = address;
        }
    }

    static inline void allocatestructure(pge_structure_t **structure, pge_entry_t *entry) {
        *structure = (pge_structure_t*) mem::allocatephys(1);
        memset(*structure, 0, sizeof(pge_structure_t));
        addattribute(entry, PGE_PRESENT_BIT | PGE_WRITABLE_BIT);
        setframeaddr(entry, (uint64_t) *structure);
    }

    pge_structure_t *getkernelpml4(bool phys) {
        return (phys) ? kernelpml4 : (pge_structure_t*) ((uintptr_t) kernelpml4 + PGE_KERNEL_VBASE);
    }

    uintptr_t findfirstfree(pge_structure_t *pml4, uintptr_t start, uintptr_t end, size_t n) {
        uintptr_t firstfree = 0;
        intmax_t reqpages = n;
        bool cached = false;

        // Set our table indexes to their initial values.
        size_t i = pml4index(start);
        size_t j = pdpeindex(start);
        size_t k = pdeindex(start);
        size_t l = pteindex(start);

        do { // Start iterating through the PML4.
            pge_entry_t *pml4e = &pml4->entries[i];
            if(testattribute(pml4e, PGE_PRESENT_BIT)) {
                // Get the address of the PDPT from the PML4E.
                pge_structure_t *pdpt = getframeaddr(pml4e);

                do { // Start iterating through the PDPT.
                    pge_entry_t *pdpe = &pdpt->entries[j];
                    if(testattribute(pdpe, PGE_PRESENT_BIT)) {
                        // If we have a huge present PDPE, reset parameters.
                        if(testattribute(pdpe, PGE_HUGEPAGE_BIT)) {
                            cached = false;
                            reqpages = n;
                            continue;
                        }

                        // Get the address of the PDT from the PDPE.
                        pge_structure_t *pdt = getframeaddr(pdpe);

                        do { // Start iterating through the PDT.
                            pge_entry_t *pde = &pdt->entries[k];
                            if(testattribute(pde, PGE_PRESENT_BIT)) {
                                // If we have a huge present PDE, reset parameters.
                                if(testattribute(pde, PGE_HUGEPAGE_BIT)) {
                                    cached = false;
                                    reqpages = n;
                                    continue;
                                }

                                // Get the address of the PT from the PDE.
                                pge_structure_t *pt = getframeaddr(pde);

                                do { // Start iterating through the PT.
                                    pge_entry_t *pte = &pt->entries[l];
                                    if(testattribute(pte, PGE_PRESENT_BIT)) {
                                        cached = false;
                                        reqpages = n;
                                        continue;
                                    } else {
                                        cacheaddress(&cached, &firstfree, i, j, k, l); // Free page!
                                        if(--reqpages <= 0) goto returner;
                                    }
                                } while(++l < PGE_ENTRIES_PER_STRUCTURE);

                            l = 0;
                            } else {
                                cacheaddress(&cached, &firstfree, i, j, k, l); // Free PDE: 512 free pages.
                                if((reqpages -= PGE_PDE_ADDRSPACE / PGE_PTE_ADDRSPACE) <= 0) goto returner;
                            }
                        } while(++k < PGE_ENTRIES_PER_STRUCTURE);

                    k = 0;
                    } else {
                        cacheaddress(&cached, &firstfree, i, j, k, l); // Free PDPE: 256k free pages.
                        if((reqpages -= PGE_PDPE_ADDRSPACE / PGE_PTE_ADDRSPACE) <= 0) goto returner;
                    }
                } while(++j < PGE_ENTRIES_PER_STRUCTURE);

            j = 0;
            } else {
                cacheaddress(&cached, &firstfree, i, j, k, l); // Free PML4E: ~134M free pages.
                if((reqpages -= PGE_PML4E_ADDRSPACE / PGE_PTE_ADDRSPACE) <= 0) goto returner;
            }
        } while(++i <= pml4index(end));

        returner:
        if(firstfree < end) return firstfree;
        return 0;
    }

    uintptr_t virt2phys(pge_structure_t *pml4, uintptr_t virt) {
        // Get the paging table indexes for this virtual address.
        size_t i = pml4index(virt);
        size_t j = pdpeindex(virt);
        size_t k = pdeindex(virt);
        size_t l = pteindex(virt);

        // Check if the PML4E is present.
        pge_entry_t *pml4e = &pml4->entries[i];
        if(!testattribute(pml4e, PGE_PRESENT_BIT)) return 0;

        // Check if the PDPE is present.
        pge_structure_t *pdpt = getframeaddr(pml4e);
        pge_entry_t *pdpe = &pdpt->entries[j];
        if(!testattribute(pdpe, PGE_PRESENT_BIT)) return 0;
        if(testattribute(pdpe, PGE_HUGEPAGE_BIT)) return (uintptr_t) getframeaddr(pdpe);

        // Check if the PDE is present.
        pge_structure_t *pdt = getframeaddr(pdpe);
        pge_entry_t *pde = &pdt->entries[k];
        if(!testattribute(pde, PGE_PRESENT_BIT)) return 0;
        if(testattribute(pde, PGE_HUGEPAGE_BIT)) return (uintptr_t) getframeaddr(pde);

        // Check if the PT is present.
        pge_structure_t *pt = getframeaddr(pde);
        pge_entry_t *pte = &pt->entries[l];
        if(!testattribute(pte, PGE_PRESENT_BIT)) return 0;
        return (uintptr_t) getframeaddr(pte);
    }

    void mappage(pge_structure_t *pml4, mem_pagetype_t type, uintptr_t virt, uintptr_t phys, uint64_t flags) {
        pge_entry_t *pml4e = &pml4->entries[pml4index(virt)];

        // If the PML4E isn't present, allocate a new PDPT and mark the PML4E as present.
        pge_structure_t *pdpt = getframeaddr(pml4e);
        if(!testattribute(pml4e, PGE_PRESENT_BIT)) allocatestructure(&pdpt, pml4e);
        pge_entry_t *pdpe = &pdpt->entries[pdpeindex(virt)];

        if(type == PGE_HUGE1GB_PAGE) {
            // Set the hugepage bit if required and return, job finished.
            addattribute(pdpe, flags | PGE_HUGEPAGE_BIT);
            setframeaddr(pdpe, phys & 0x000FFFFFC0000000);
            return;
        }

        // If the PDPE isn't present, allocate a new PDT and mark the PDPE as present.
        pge_structure_t *pdt = getframeaddr(pdpe);
        if(!testattribute(pdpe, PGE_PRESENT_BIT)) allocatestructure(&pdt, pdpe);
        pge_entry_t *pde = &pdt->entries[pdeindex(virt)];

        if(type == PGE_HUGE2MB_PAGE) {
            // Set the hugepage bit if required and return, job finished.
            addattribute(pde, flags | PGE_HUGEPAGE_BIT);
            setframeaddr(pde, phys & 0x000FFFFFFFE00000);
            return;
        }

        // If the PDE isn't present, allocate a new page table and mark the PDE as present.
        pge_structure_t *pt = getframeaddr(pde);
        if(!testattribute(pde, PGE_PRESENT_BIT)) allocatestructure(&pt, pde);
        pge_entry_t *pte = &pt->entries[pteindex(virt)];

        if(type == PGE_REGULAR_PAGE) {
            addattribute(pte, flags);
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
        for(uintptr_t v = virt; v < vmax; v += 0x1000, phys += 0x1000) mappage(pml4, PGE_REGULAR_PAGE, v, phys, PGE_PRESENT_BIT | PGE_WRITABLE_BIT);
        return (void*) virt;
    }

    void *allocatemmio(uintptr_t phys, size_t n) {
        // Cache the page-aligned address and the offset.
        uintptr_t palign = phys & 0xFFFFFFFFFFFFF000;
        uintptr_t offset = phys & 0xFFF;

        // Check the MMIO address space for already mapped addresses.
        for(uintptr_t cursor = PGE_MMIO_BASEADDR; cursor < PGE_MMIO_ENDADDR; cursor += 0x1000) {
            uintptr_t mapped = virt2phys(kernelpml4, cursor);
            if(mapped != 0 && mapped == palign) return (void*) (cursor + offset);
        }

        // Find some free space in the kernel's address space if it hasn't already been mapped.
        uintptr_t virt = findfirstfree(kernelpml4, PGE_MMIO_BASEADDR, PGE_MMIO_ENDADDR, n);
        uintptr_t vmax = virt + (n * PGE_PTE_ADDRSPACE);
        if(virt == 0) return nullptr;

        for(uintptr_t v = virt; v < vmax; v += 0x1000, palign += 0x1000) mappage(kernelpml4, PGE_REGULAR_PAGE, v, palign, PGE_PRESENT_BIT | PGE_WRITABLE_BIT | PGE_UNCACHEABLE_BIT);
        return (void*) (virt + offset);
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

        ctxpanic(regs, "page fault exception, %s\n"
                       "[kpanic] cr2 (page address): %p\n"
                       "[kpanic] cr3 (PML4 address): %p",
                       reason, cr2, cr3);
    }

    void initialisevirt(void) {
        // Identity map the first two megabytes except for 0x0.
        for(uintptr_t p = 0x1000; p < 0x200000; p += 0x1000) {
            mappage(kernelpml4, PGE_REGULAR_PAGE, p, p, PGE_PRESENT_BIT | PGE_WRITABLE_BIT);
        }

        // Map the kernel into the higher half of the VAS.
        for(uintptr_t p = 0x100000, v = p + PGE_KERNEL_VBASE; v < (uintptr_t) &kernelend; p += 0x1000, v += 0x1000) {
            mappage(kernelpml4, PGE_REGULAR_PAGE, v, p, PGE_PRESENT_BIT | PGE_WRITABLE_BIT);
        }

        // Map the framebuffer into the kernel's address space.
        uintptr_t pend = mboot::info.fbinfo->common.framebuffer + (mboot::info.fbinfo->common.height * mboot::info.fbinfo->common.width * (mboot::info.fbinfo->common.bpp / 8));
        for(uintptr_t p = mboot::info.fbinfo->common.framebuffer, v = 0xFFFFFFFFC0000000; p < pend; p += 0x1000, v += 0x1000) {
            mappage(kernelpml4, PGE_REGULAR_PAGE, v, p, PGE_PRESENT_BIT | PGE_WRITABLE_BIT);
        }

        // Register mem::pfhandler() as the page fault handler.
        idt::registerhandler(14, &pfhandler);
        currentpml4 = kernelpml4;

        // Set the framebuffer's address and reload CR3.
        mboot::info.fbinfo->common.framebuffer = 0xFFFFFFFFC0000000;
        writecr3((uintptr_t) kernelpml4);
    }
}
