#ifndef FREELSD_KERNEL_ACPI_HEADER
#define FREELSD_KERNEL_ACPI_HEADER

#include <stddef.h>
#include <stdint.h>

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

    typedef struct rsdt {
        sdthdr_t header;
        uint32_t pointers[1];
    } __attribute__((packed)) rsdt_t;

    typedef struct xsdt {
        sdthdr_t header;
        uint64_t pointers[1];
    } __attribute__((packed)) xsdt_t;

    typedef struct descriptor {
        uintptr_t msdt;
        uint8_t revision;
    } rdsc_t;

    // Root system description pointer.
    extern rsdp_t *rsdptr;

    // Initialise the ACPI subsystem.
    void initialise(void);
}

#endif
