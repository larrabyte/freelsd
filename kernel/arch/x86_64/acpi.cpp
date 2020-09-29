#include <mem/alloc.hpp>
#include <mem/libc.hpp>
#include <mem/virt.hpp>
#include <logger.hpp>
#include <errors.hpp>
#include <apic.hpp>
#include <acpi.hpp>

namespace acpi {
    uint8_t processors[ACPI_MAX_PROCESSORS];
    rsdp_t *rsdptr = nullptr;
    size_t cpucount = 0;
    rsdc_t descriptor;

    static void parsemadt(void) {
        // Get the address of the MADT (multiple APIC description table).
        madt_t *madt = (madt_t*) findsdt("APIC");
        if(!madt) panic("system does not have an MADT!");

        // Zero out the processor array and calculate the start of the MADT entries.
        memset(processors, 0, sizeof(uint8_t) * ACPI_MAX_PROCESSORS);
        madt_entry_t *final = (madt_entry_t*) ((uintptr_t) madt + madt->header.length);
        madt_entry_t *start = (madt_entry_t*) ((uintptr_t) madt + sizeof(madt_t));
        apic::isos.count = apic::nmis.count = 0;

        // Perform a first sweep of the MADT to determine local APICs, I/O APICs and the number of ISOs and NMIs.
        for(madt_entry_t *cursor = start; cursor < final; cursor = (madt_entry_t*) ((uintptr_t) cursor + cursor->length)) {
            if(cursor->type == 0) { // Type 0 specifies a local APIC.
                madt_entry_localapic_t *local = (madt_entry_localapic_t*) cursor;
                processors[cpucount++] = local->apicid;
            } else if(cursor->type == 1) { // Type 1 specifies an I/O APIC.
                madt_entry_ioapic_t *io = (madt_entry_ioapic_t*) cursor;
                apic::iobase = io->address;
            } else if(cursor->type == 2) { // Type 2 specifies an interrupt source override.
                apic::isos.count++;
            } else if(cursor->type == 4) { // Type 4 specifies a non-maskable interrupt.
                apic::nmis.count++;
            }
        }

        // Perform a second sweep to create and fill the ISO/NMI arrays.
        apic::isos.pointers = (madt_entry_iso_t**) kmalloc(apic::isos.count * sizeof(uintptr_t));
        apic::nmis.pointers = (madt_entry_nmi_t**) kmalloc(apic::nmis.count * sizeof(uintptr_t));
        size_t isoindex = 0, nmiindex = 0;

        for(madt_entry_t *cursor = start; cursor < final; cursor = (madt_entry_t*) ((uintptr_t) cursor + cursor->length)) {
            if(cursor->type == 2) apic::isos.pointers[isoindex++] = (madt_entry_iso_t*) cursor;
            if(cursor->type == 4) apic::nmis.pointers[nmiindex++] = (madt_entry_nmi_t*) cursor;
        }

        log::info("[osacpi] system MADT identified %d processors.\n\n", cpucount);
    }

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
        // We need ACPI, panic if Multiboot2 couldn't find a RSDP.
        if(rsdptr == nullptr) panic("system is non-ACPI compliant!");

        // Calculate regular and extended checksums for the RSDP.
        uint8_t c1 = checksum((char*) rsdptr, 20);
        uint8_t c2 = checksum((char*) rsdptr, 36);
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
        // No idea why we need a 2-byte offset to read the addresses properly.
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
        }

        parsemadt();
    }
}
