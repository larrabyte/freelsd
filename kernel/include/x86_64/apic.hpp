#ifndef FREELSD_KERNEL_APIC_HEADER
#define FREELSD_KERNEL_APIC_HEADER

#include <acpi.hpp>
#include <stdint.h>

// Calculate the offset of a given IRQ in the IOREDTBL.
#define ioredtbl(irq) ((apic::registers_t) (0x10 + 2 * irq))

namespace apic {
    typedef enum registers {
        APIC_BASEADDRESS_MSR            = 0x01B,
        LAPIC_IDENTIFIER_REG            = 0x020,
        LAPIC_VERSION_REG               = 0x030,
        LAPIC_TASK_PRIORITY_REG         = 0x080,
        LAPIC_ARBITRATION_PRIORITY_REG  = 0x090,
        LAPIC_PROCESSOR_PRIORITY_REG    = 0x0A0,
        LAPIC_END_OF_INTERRUPT_REG      = 0x0B0,
        LAPIC_REMOTE_READ_REG           = 0x0C0,
        LAPIC_LOGICAL_DESTINATION_REG   = 0x0D0,
        LAPIC_DESTINATION_FORMAT_REG    = 0x0E0,
        LAPIC_SPURIOUS_INTERRUPT_REG    = 0x0F0,
        LAPIC_ERROR_STATUS_REG          = 0x280,
        LAPIC_LOW_INT_COMMAND_REG       = 0x300,
        LAPIC_HIGH_INT_COMMAND_REG      = 0x310,
        LAPIC_TIMER_LVT_ENTRY_REG       = 0x320,
        LAPIC_THERMAL_LVT_ENTRY_REG     = 0x330,
        LAPIC_PERFCOUNT_LVT_ENTRY_REG   = 0x340,
        LAPIC_LINT0_VECTOR_ENTRY_REG    = 0x350,
        LAPIC_LINT1_VECTOR_ENTRY_REG    = 0x360,
        LAPIC_ERROR_VECTOR_ENTRY_REG    = 0x370,
        LAPIC_TIMER_INITIAL_COUNT_REG   = 0x380,
        LAPIC_TIMER_CURRENT_COUNT_REG   = 0x390,
        LAPIC_TIMER_DIVIDE_CONFIG_REG   = 0x3E0,
        LAPIC_EXTENDED_FEATURE_REG      = 0x400,
        LAPIC_EXTENDED_CONTROL_REG      = 0x410,
        LAPIC_SPECIFIC_EOI_REG          = 0x420,
        IOAPIC_IDENTIFIER_REG           = 0x000,
        IOAPIC_VERSION_REG              = 0x001,
        IOAPIC_ARBITRATION_REG          = 0x002
    } registers_t;

    typedef enum flags {
        APIC_DELIVERY_FIXED             = 0b000 << 8,
        APIC_DELIVERY_LOWEST            = 0b001 << 8,
        APIC_DELIVERY_SMI               = 0b010 << 8,
        APIC_DELIVERY_NMI               = 0b100 << 8,
        APIC_DELIVERY_INIT              = 0b101 << 8,
        APIC_DELIVERY_EXTINT            = 0b111 << 8,
        APIC_DESTINATION_PHYSICAL       = 0 << 11,
        APIC_DESTINATION_LOGICAL        = 1 << 11,
        APIC_POLARITY_HIGH              = 0 << 13,
        APIC_POLARITY_LOW               = 1 << 13,
        APIC_TRIGGER_EDGE               = 0 << 15,
        APIC_TRIGGER_LEVEL              = 1 << 15,
        APIC_INTERRUPT_UNMASKED         = 0 << 16,
        APIC_INTERRUPT_MASKED           = 1 << 16
    } flags_t;

    typedef struct ioredtbl_entry {
        // Construct an IOREDTBL entry using a 64-bit integer.
        ioredtbl_entry(uint64_t flags) {
            destination                 = (flags >> 56) & 0xFF;
            masked                      = (flags >> 16) & 0x01;
            trigger                     = (flags >> 15) & 0x01;
            remoteirr                   = (flags >> 14) & 0x01;
            polarity                    = (flags >> 13) & 0x01;
            status                      = (flags >> 12) & 0x01;
            destmode                    = (flags >> 11) & 0x01;
            deliverymode                = (flags >> 10) & 0x07;
            vector                      = flags & 0xFF;
            reservedhi                  = 0;
        }

        // Convert an IOREDTBL structure into a 64-bit integer.
        operator uint64_t() const {
            return (uint64_t) destination << 56 | masked << 16 | trigger << 15 | remoteirr << 14  |
                   polarity << 13 | status << 12 | destmode << 11 | deliverymode << 10 | vector;
        }

        uint64_t destination            : 8;
        uint64_t reservedhi             : 39;
        uint64_t masked                 : 1;
        uint64_t trigger                : 1;
        uint64_t remoteirr              : 1;
        uint64_t polarity               : 1;
        uint64_t status                 : 1;
        uint64_t destmode               : 1;
        uint64_t deliverymode           : 3;
        uint64_t vector                 : 8;
    } ioredtbl_entry_t;

    typedef struct isolist {
        acpi::madt_entry_iso_t **isos;
        size_t count;
    } apic_config_iso_t;

    typedef struct nmilist {
        acpi::madt_entry_nmi_t **nmis;
        size_t count;
    } apic_config_nmi_t;

    extern apic_config_iso_t isolist;
    extern apic_config_nmi_t nmilist;
    extern uintptr_t localbase;
    extern uintptr_t iobase;

    // Read from a current processor's APIC register.
    uint32_t readlocal(registers_t index);

    // Write to the current processor's APIC register.
    void writelocal(registers_t index, uint32_t value);

    // Read a register from the I/O APIC.
    uint64_t readio(registers_t index, bool read64);

    // Write to a register in the I/O APIC.
    void writeio(registers_t index, uint64_t value, bool write64);

    // Redirect an IRQ to an interrupt vector on a given processor in the I/O APIC.
    void redirectio(uint8_t irq, ioredtbl_entry_t entry);

    // Set an IRQ mask on the I/O APIC.
    void setmaskio(uint8_t irq, bool mask);

    // Enable the current procesor's local APIC.
    void enablelocal(void);

    // Run by the bootstrap processor to perform initial APIC setup.
    void initialisebsp(void);
}

#endif
