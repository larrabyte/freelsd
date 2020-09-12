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

    void initialise(void) {
        uint8_t c1 = checksum((char*) rsdptr, 20);
        uint8_t c2 = checksum((char*) rsdptr, 36);

        // Make sure the checksums are valid before proceeding.
        if(c1 != 0) panic("RSDP table checksum not valid! 0x%hx != 0x00.", c1);
        if(rsdptr->revision == 2 && c2 != 0) panic("RSDP extended table checksum not valid! 0x%hx != 0x00.", c2);

        // Make sure to set the correct system descriptor table address in the ACPI descriptor.
        descriptor.msdt = (rsdptr->revision == 0) ? rsdptr->rsdtaddr : rsdptr->xsdtaddr;
        descriptor.msdt = (uintptr_t) mem::allocatemmio(descriptor.msdt);
        descriptor.revision = rsdptr->revision;

        // Create a new array to store ACPI table pointers.
        sdthdr_t *msdt = (sdthdr_t*) descriptor.msdt;
        size_t entries = (msdt->length - sizeof(sdthdr_t)) / 4;
        if(msdt->revision == 2) entries /= 2;
        uintptr_t *pointers = (uintptr_t*) kmalloc(entries * 8);

        if(descriptor.revision == 0) {
            uint32_t *tables = (uint32_t*) ((uintptr_t) msdt + sizeof(sdthdr_t) + 2);
            for(size_t i = 0; i < entries; i++) pointers[i] = (uintptr_t) tables[i];
        }

        else if(descriptor.revision == 2) {
            uint64_t *tables = (uint64_t*) ((uintptr_t) msdt + sizeof(sdthdr_t) + 2);
            for(size_t i = 0; i < entries; i++) pointers[i] = (uintptr_t) tables[i];
        }

        log::info("[osacpi] central SDT address: %p (%d bytes, rev %hd)\n", msdt, msdt->length, descriptor.revision);
        for(size_t i = 0; i < entries; i++) {
            pointers[i] = (uintptr_t) mem::allocatemmio(pointers[i]);
            log::info("[osacpi] SDT table addresses: %p\n", pointers[i]);
        } log::info("\n");
    }
}
