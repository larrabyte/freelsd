#include <mem/virt.hpp>
#include <mem/phys.hpp>
#include <mem/libc.hpp>
#include <serial.hpp>
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

    inline uint32_t getframeaddr(uint32_t entry) {
        return entry & PGE_FRAME_BITS;
    }

    void pfhandler(idt::regs32_t *regs) {
        // Fetch the faulty address from control register 2 (get EIP).
        uint32_t eip; asm volatile("mov %%cr2, %0" : "=r" (eip));

        panic("page fault exception generated.\n"
              "[kernel] fault address (eip): 0x%p\n"
              "[kernel] page no-execute bit: %s\n"
              "[kernel] page supervisor bit: %s\n"
              "[kernel] page writable bit: %s\n"
              "[kernel] page reserved bit: %s\n"
              "[kernel] page present bit: %s", eip,
              (checkbit(regs->errcode, 4)) ? "set" : "unset",
              (checkbit(regs->errcode, 2)) ? "set" : "unset",
              (checkbit(regs->errcode, 1)) ? "set" : "unset",
              (checkbit(regs->errcode, 3)) ? "set" : "unset",
              (checkbit(regs->errcode, 0)) ? "set" : "unset");
    }

    pt_entry_t *lookupentry(uint32_t virtaddr) {
        pd_entry_t *dentry = &currentdir->entries[pdeindex(virtaddr)];
        pt_table_t *tentry = (pt_table_t*) getframeaddr((uint32_t) dentry);
        return (pt_entry_t*) &tentry->entries[pteindex(virtaddr)];
    }

    bool allocpage(pt_entry_t *entry) {
        // Allocate a free block of physical memory.
        void *block = physmem::allocblocks(1);
        if(!block) return false;

        // Map it to the page table entry passed in.
        addattribute(entry, PTE_PRESENT_BIT);
        setframe(entry, (uint32_t) block);
        return true;
    }

    void freepage(pt_entry_t *entry) {
        // Get physical block address and free if it exists.
        void *block = (void*) getframeaddr(*entry);
        if(block) physmem::freeblocks((uint32_t) block, 1);

        // Unset the present bit in the page.
        delattribute(entry, PTE_PRESENT_BIT);
    }

    void mappage(uint32_t phys, uint32_t virt) {
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
            table = (pt_table_t*) getframeaddr(*pde);
        }

        // Fetch the address of the page table entry and map it in.
        pt_entry_t *page = &table->entries[pteindex(virt)];
        addattribute(page, PTE_PRESENT_BIT | PTE_WRITABLE_BIT);
        setframe(page, phys);
    }

    void initialise(void) {
        // Allocate a kernel page directory and zero it out.
        kernelpd = (pd_directory_t*) physmem::allocblocks(1);
        memset(kernelpd, 0, sizeof(pd_directory_t));
        currentdir = kernelpd;

        // Identity map the first megabyte of physical memory.
        for(size_t firstone = 0x0; firstone < 0x100000; firstone += 0x1000) mappage(firstone, firstone);

        // Map sixteen megabytes from 0x100000 to 0xC0000000 (virtual address of 3GB).
        for(size_t kernelphys = 0x100000; kernelphys < 0x1100000; kernelphys += 0x1000) mappage(kernelphys, kernelphys + 0xC0000000);

        // Identity map the last 64 megabytes (minus 4K) of physical memory for the framebuffer.
        for(size_t framebuffer = 0xFD000000; framebuffer < 0xFF000000; framebuffer += 0x1000) mappage(framebuffer, framebuffer);

        // Register the page fault handler and switch the active page directory.
        idt::registerhandler(14, &pfhandler);
        loadpdbr((uint32_t) currentdir);
    }
}
