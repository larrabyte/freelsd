#ifndef FREELSD_KERNEL_ACPI_HEADER
#define FREELSD_KERNEL_ACPI_HEADER

#include <stddef.h>
#include <stdint.h>

#define ACPI_MAX_PROCESSORS 256

namespace acpi {
    typedef struct rsdp {
        char signature[8];
        uint8_t checksum;
        char oemid[6];
        uint8_t revision;
        uint32_t rsdtaddr;
        uint32_t xlength;
        uint64_t xsdtaddr;
        uint8_t xchecksum;
        uint8_t xreserved[3];
    } __attribute__((packed)) rsdp_t;

    typedef struct sdtheader {
        char signature[4];
        uint32_t length;
        uint8_t revision;
        uint8_t checksum;
        char oemid[6];
        char tableid[6];
        uint32_t oemrevision;
        uint32_t creatorid;
        uint32_t creatorrev;
    } __attribute__((packed)) sdthdr_t;

    typedef struct madt_entry {
        uint8_t type;
        uint8_t length;
    } __attribute__((packed)) madt_entry_t;

    typedef struct madt_localapic {
        madt_entry_t header;
        uint8_t procid;
        uint8_t apicid;
        uint32_t flags;
    } __attribute__((packed)) madt_entry_localapic_t;

    typedef struct madt_ioapic {
        madt_entry_t header;
        uint8_t id;
        uint8_t reserved;
        uint32_t address;
        uint32_t gsib;
    } __attribute__((packed)) madt_entry_ioapic_t;

    typedef struct madt_iso {
        madt_entry_t header;
        uint8_t sourcebus;
        uint8_t sourceirq;
        uint32_t gsi;
        uint16_t flags;
    } __attribute__((packed)) madt_entry_iso_t;

    typedef struct madt_nmi {
        madt_entry_t header;
        uint8_t procid;
        uint16_t flags;
        uint8_t lint;
    } __attribute__((packed)) madt_entry_nmi_t;

    typedef struct madt_apic_override {
        uint16_t reserved;
        uint64_t address;
    } __attribute__((packed)) madt_entry_override_t;

    typedef struct madt {
        sdthdr_t header;
        uint32_t localapic;
        uint32_t flags;
    } madt_t;

    typedef struct descriptor {
        uintptr_t msdt;
        uint8_t revision;
        size_t count;
        uintptr_t *pointers;
    } rsdc_t;

    // Calculate an ACPI table's checksum.
    uint8_t checksum(char *address, size_t length);

    // Return a pointer to an ACPI SDT given a signature.
    void *findsdt(const char *signature);

    // Initialise the ACPI subsystem.
    void initialise(void);
}

#endif
