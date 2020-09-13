#include <multiboot.hpp>
#include <mem/alloc.hpp>
#include <mem/libc.hpp>
#include <mem/virt.hpp>
#include <logger.hpp>
#include <errors.hpp>
#include <acpi.hpp>

namespace acpi {
    rdsc_t descriptor;
    rsdp_t *rsdptr;

    uint8_t checksum(char *address, size_t length) {
        char *end = address + length;
        uint8_t sum = 0;

        while(address < end) sum += *(address++);
        return sum;
    }

    void *findsdt(const char *signature) {
        for(size_t i = 0; i < descriptor.count; i++) {
            sdthdr_t *sdt = (sdthdr_t*) descriptor.pointers[i];
            if(memcmp(signature, sdt->signature, 4) == 0) return sdt;
        }

        return nullptr;
    }

    void initialise(void) {
        uint8_t c1 = checksum((char*) rsdptr, 20);
        uint8_t c2 = checksum((char*) rsdptr, 36);

        // Make sure the checksums are valid before proceeding.
        if(c1 != 0) panic("RSDP table checksum not valid! 0x%hhx != 0x00.", c1);
        if(rsdptr->revision == 2 && c2 != 0) panic("RSDP extended table checksum not valid! 0x%hhx != 0x00.", c2);

        // Make sure to set the correct system descriptor table address in the ACPI descriptor.
        descriptor.msdt = (rsdptr->revision == 0) ? rsdptr->rsdtaddr : rsdptr->xsdtaddr;
        descriptor.msdt = (uintptr_t) mem::allocatemmio(descriptor.msdt, 1);
        descriptor.revision = rsdptr->revision;

        // Calculate address sizes and count.
        sdthdr_t *msdt = (sdthdr_t*) descriptor.msdt;
        uint64_t signature = (descriptor.revision == 0) ? 4 : 8;
        descriptor.count = (msdt->length - sizeof(sdthdr_t)) / signature;

        // Allocate an array on the heap to store the ACPI table pointers.
        descriptor.pointers = (uintptr_t*) kmalloc(descriptor.count * 8);
        memset(descriptor.pointers, 0, descriptor.count * 8);

        // Copy data from the RSDT/XSDT into the new table pointer array.
        // No idea why I need a 2-byte offset to read the addresses properly.
        char *tabledata = (char*) ((uintptr_t) msdt + sizeof(sdthdr_t) + 2);
        for(size_t i = 0; i < descriptor.count; i++) memcpy(&descriptor.pointers[i], &tabledata[i * signature], signature);
        signature = 0;

        // Print out system descriptor table information.
        memcpy(&signature, msdt->signature, 4);
        log::info("[osacpi] %s address: %p (%d bytes)\n", &signature, msdt, msdt->length);

        for(size_t i = 0; i < descriptor.count; i++) {
            // Allocate an initial page to access the standard table header.
            sdthdr_t *table = (sdthdr_t*) mem::allocatemmio(descriptor.pointers[i], 1);
            if(table->length > PGE_PTE_ADDRSPACE) mem::allocatemmio(descriptor.pointers[i] + PGE_PTE_ADDRSPACE, table->length / PGE_PTE_ADDRSPACE);

            // Set the new table pointer to the allocated virtual address.
            descriptor.pointers[i] = (uintptr_t) table;
            memcpy(&signature, table->signature, 4);
            log::info("[osacpi] %s address: %p (%d bytes)\n", &signature, table, table->length);
        } log::info("\n");
    }
}
