#ifndef FREELSD_KERNEL_APIC_HEADER
#define FREELSD_KERNEL_APIC_HEADER

#include <acpi.hpp>
#include <stdint.h>

#define APIC_BASE_MSR 0x1B

namespace apic {
    typedef enum localregs {
        LAPIC_IDENTIFIER_REG            = 0x20,
        LAPIC_VERSION_REG               = 0x30,
        LAPIC_TASK_PRIORITY_REG         = 0x80,
        LAPIC_ARBITRATION_PRIORITY_REG  = 0x90,
        LAPIC_PROCESSOR_PRIORITY_REG    = 0xA0,
        LAPIC_END_OF_INTERRUPT_REG      = 0xB0,
        LAPIC_REMOTE_READ_REG           = 0xC0,
        LAPIC_LOGICAL_DESTINATION_REG   = 0xD0,
        LAPIC_DESTINATION_FORMAT_REG    = 0xE0,
        LAPIC_SPURIOUS_INTERRUPT_REG    = 0xF0,
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
        LAPIC_SPECIFIC_EOI_REG          = 0x420
    } localregs_t;

    typedef enum ioregs {
        IOAPIC_IDENTIFIER_REG,
        IOAPIC_VERSION_REG,
        IOAPIC_ARBITRATION_REG
    } ioregs_t;

    typedef enum iodeliverymode {
        IOAPIC_DELIVERY_FIXED,
        IOAPIC_DELIVERY_LOWEST,
        IOAPIC_DELIVERY_SMI,
        IOAPIC_DELIVERY_RESERVED1,
        IOAPIC_DELIVERY_NMI,
        IOAPIC_DELIVERY_INIT,
        IOAPIC_DELIVERY_RESERVED2,
        IOAPIC_DELIVERY_EXTERNAL
    } io_deliverymode_t;

    typedef struct isolist {
        acpi::madt_entry_iso_t **isos;
        size_t count;
    } apic_config_iso_t;

    typedef struct nmilist {
        acpi::madt_entry_nmi_t **nmis;
        size_t count;
    } apic_config_nmi_t;

    // APIC ISO/NMI lists.
    extern apic_config_iso_t isolist;
    extern apic_config_nmi_t nmilist;

    // Defines the local and I/O APIC base addresses.
    extern uintptr_t localbase;
    extern uintptr_t iobase;

    // Read from a current processor's APIC register.
    uint32_t readlocal(localregs_t index);

    // Write to the current processor's APIC register.
    void writelocal(localregs_t index, uint32_t value);

    // Read a register from the I/O APIC.
    uint64_t readio(uint8_t index, bool read64);

    // Write to a register in the I/O APIC.
    void writeio(uint8_t index, uint64_t value, bool write64);

    // Redirect an IRQ to an interrupt vector on a given processor in the I/O APIC.
    void redirectio(uint8_t irq, uint8_t vector, uint8_t id, io_deliverymode_t mode);

    // Enable the current procesor's local APIC.
    void enablelocal(void);

    // Run by the bootstrap processor to perform initial APIC setup.
    void initialisebsp(void);
}

#endif
