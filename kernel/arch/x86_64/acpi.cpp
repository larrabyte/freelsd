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
        descriptor.msdt = (uintptr_t) mem::allocatemmio(descriptor.msdt, 1);
        descriptor.revision = rsdptr->revision;

        if(descriptor.revision == 0) {
            rsdt_t *rsdt = (rsdt_t*) descriptor.msdt;
            size_t entries = (rsdt->header.length - sizeof(sdthdr_t)) / 4;
            uint32_t *tables = (uint32_t*) ((uintptr_t) rsdt->pointers + 2);
            log::info("[osacpi] rsdt address: %p (%d bytes)\n", rsdt, rsdt->header.length);

            for(size_t i = 0; i < entries; i++) {
                log::info("[osacpi] rsdt entry %zd: %p\n", i, (uintptr_t) tables[i]);
            } log::info("\n");
        }

        else if(descriptor.revision == 2) {
            xsdt_t *xsdt = (xsdt_t*) descriptor.msdt;
            size_t entries = (xsdt->header.length - sizeof(sdthdr_t)) / 8;
            uint64_t *tables = (uint64_t*) ((uintptr_t) xsdt->pointers + 2);
            log::info("[osacpi] xsdt address: %p (%d bytes)\n", xsdt, xsdt->header.length);

            for(size_t i = 0; i < entries; i++) {
                log::info("[osacpi] xsdt entry %zd: %p\n", i, (uintptr_t) tables[i]);
            } log::info("\n");
        }
    }
}
