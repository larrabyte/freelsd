/*  Paging information and data structures.
    ---------------------------------------
    Paging data structure layout:
        pml4_table_t:          --> Page Map Level 4 abstraction, manages 256TB of address space.
            pml4_entry_t[512]  --> 512 page map level 4 entries.

        pdp_table_t:           --> Page directory pointer abstraction, manages 512GB of address space.
            pdp_entry_t[512]   --> 512 page directory pointer entries.

        pd_directory_t:        --> Page directory abstraction, manages 1GB of address space.
            pd_entry_t[512]    --> 512 page directory entries.

        pt_table_t:            --> Page table abstraction, manages 2MB of address space.
            pt_entry_t[512]    --> 512 page table entries.

    Paging entry formats:
        pml4_entry_t:          --> Page Map Level 4 entry, manages 512GB of address space.
            pdp_table_t*       --> Address of a page directory pointer table along with flags.

        pdp_entry_t:           --> Page directory pointer entry, manages 1GB of address space.
            pd_directory_t*    --> Address of a page directory along with flags.

        pd_entry_t:            --> Page directory entry, manages 2MB of address space.
            pt_table_t*        --> Address of a page table along with flags.

        pt_entry_t:            --> Page table entry, manages 4KB of address space.
            uint64_t           --> Address of physical page along with flags.

    Format of a virtual address:
        0xAAAAAAAAAAAAAAAA BBBBBBBBB CCCCCCCCC DDDDDDDDD EEEEEEEEE FFFFFFFFFFFF
            A: Sign extension of bit 47       (bits 48-63)
            B: Page Map Level 4 offset        (bits 39-47)
            C: Page Directory Pointer offset  (bits 30-38)
            D: Page Directory offset          (bits 21-29)
            E: Page table offset              (bits 12-20)
            F: Physical address offset        (bits 00-11)
*/
