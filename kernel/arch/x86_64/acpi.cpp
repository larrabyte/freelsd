#include <multiboot.hpp>
#include <mem/alloc.hpp>
#include <mem/libc.hpp>
#include <mem/virt.hpp>
#include <logger.hpp>
#include <errors.hpp>
#include <acpi.hpp>

namespace acpi {
    char signature[5] = "FLSD";
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
        if(c1 != 0) panic("RSDP table checksum not valid! 0x%hhx != 0x00.", c1);
        if(rsdptr->revision == 2 && c2 != 0) panic("RSDP extended table checksum not valid! 0x%hhx != 0x00.", c2);

        // Make sure to set the correct system descriptor table address in the ACPI descriptor.
        descriptor.msdt = (rsdptr->revision == 0) ? rsdptr->rsdtaddr : rsdptr->xsdtaddr;
        descriptor.msdt = (uintptr_t) mem::allocatemmio(descriptor.msdt);
        descriptor.revision = rsdptr->revision;

        // Create a new array to store ACPI table pointers.
        sdthdr_t *msdt = (sdthdr_t*) descriptor.msdt;
        descriptor.count = (msdt->length - sizeof(sdthdr_t)) / 4;
        if(descriptor.revision == 2) descriptor.count /= 2;
        descriptor.pointers = (uintptr_t*) kmalloc(descriptor.count * 8);

        if(descriptor.revision == 0) {
            uint32_t *tables = (uint32_t*) ((uintptr_t) msdt + sizeof(sdthdr_t) + 2);
            for(size_t i = 0; i < descriptor.count; i++) descriptor.pointers[i] = (uintptr_t) tables[i];
        } else if(descriptor.revision == 2) {
            uint64_t *tables = (uint64_t*) ((uintptr_t) msdt + sizeof(sdthdr_t) + 2);
            for(size_t i = 0; i < descriptor.count; i++) descriptor.pointers[i] = (uintptr_t) tables[i];
        }

        // Copy the main table signature before logging it.
        memcpy(signature, msdt->signature, 4);
        log::info("[osacpi] %s address: %p (%d bytes)\n", signature, msdt, msdt->length);

        for(size_t i = 0; i < descriptor.count; i++) {
            descriptor.pointers[i] = (uintptr_t) mem::allocatemmio(descriptor.pointers[i]);
            memcpy(signature, ((sdthdr_t*) descriptor.pointers[i])->signature, 4);
            log::info("[osacpi] %s address: %p (%d bytes)\n", signature, descriptor.pointers[i], ((sdthdr_t*) descriptor.pointers[i])->length);
        } log::info("\n");
    }
}
