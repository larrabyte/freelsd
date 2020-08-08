#include <multiboot.hpp>
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

        // Set the correct system descriptor table address in the RSDP.
        descriptor.mainsdt = (rsdptr->revision == 0) ? rsdptr->rsdtaddr : rsdptr->xsdtaddr;
        descriptor.revision = rsdptr->revision;

        // Copy the OEMID and convert it into a C-style string.
        memcpy(&descriptor.oemid, &rsdptr->oemid, 6);
        descriptor.oemid[6] = '\0';

        log::info("[osacpi] mainsdt: %p\n", descriptor.mainsdt);
        log::info("[osacpi] OEM vendor: %s\n\n", descriptor.oemid);
    }
}
