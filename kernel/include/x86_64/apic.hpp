#ifndef FREELSD_KERNEL_APIC_HEADER
#define FREELSD_KERNEL_APIC_HEADER

#include <acpi.hpp>
#include <stdint.h>

// Calculate the offset of a given IRQ in the IOREDTBL.
#define ioindex(irq) ((apic::registers_t) (0x10 + 2 * irq))

// Some cool definitions!
#define APIC_DSHTYPE_IGNORE             0b00
#define APIC_DSHTYPE_SELF               0b01
#define APIC_DSHTYPE_ALL                0b10
#define APIC_DSHTYPE_OTHER              0b11
#define APIC_MSGTYPE_FIXED              0b000
#define APIC_MSGTYPE_LOWEST             0b001
#define APIC_MSGTYPE_SMI                0b010
#define APIC_MSGTYPE_NMI                0b100
#define APIC_MSGTYPE_INIT               0b101
#define APIC_MSGTYPE_STARTUP            0b110
#define APIC_MSGTYPE_EXTERNAL           0b111
#define APIC_INTMASK_BIT                1UL << 16

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

    typedef struct ioredtbl {
        // Construct an IOREDTBL entry using a 64-bit integer.
        ioredtbl(uint64_t flags) {
            destination   = (flags >> 56) & 0xFF;
            masked        = (flags >> 16) & 0x01;
            trigger       = (flags >> 15) & 0x01;
            remoteirr     = (flags >> 14) & 0x01;
            polarity      = (flags >> 13) & 0x01;
            status        = (flags >> 12) & 0x01;
            destmode      = (flags >> 11) & 0x01;
            deliverymode  = (flags >> 10) & 0x07;
            vector        = flags & 0xFF;
            reservedhi    = 0;
        }

        // Convert an IOREDTBL structure into a 64-bit integer.
        operator uint64_t() const {
            return (uint64_t) destination << 56 | masked << 16 | trigger << 15 | remoteirr << 14  |
                   polarity << 13 | status << 12 | destmode << 11 | deliverymode << 10 | vector;
        }

        uint64_t destination   : 8;
        uint64_t reservedhi    : 39;
        uint64_t masked        : 1;
        uint64_t trigger       : 1;
        uint64_t remoteirr     : 1;
        uint64_t polarity      : 1;
        uint64_t status        : 1;
        uint64_t destmode      : 1;
        uint64_t deliverymode  : 3;
        uint64_t vector        : 8;
    } ioredtbl_t;

    typedef struct lvt {
        // Construct an LVT register using a 32-bit integer.
        lvt(uint32_t flags) {
            timermode    = (flags >> 17) & 0x01;
            masked       = (flags >> 16) & 0x01;
            trigger      = (flags >> 15) & 0x01;
            remoteirr    = (flags >> 14) & 0x01;
            status       = (flags >> 12) & 0x01;
            messagetype  = (flags >> 8) & 0x07;
            vector       = flags & 0xFF;
            reservedhi   = 0;
            reservedmed  = 0;
            reservedlow  = 0;
        }

        // Convert an LVT structure into a 32-bit integer.
        operator uint32_t() const {
            return timermode << 17 | masked << 16 | trigger << 15 | remoteirr << 14 | 
                   status << 12 | messagetype << 8 | vector;
        }

        uint32_t reservedhi   : 15;
        uint32_t timermode    : 1;
        uint32_t masked       : 1;
        uint32_t trigger      : 1;
        uint32_t remoteirr    : 1;
        uint32_t reservedmed  : 1;
        uint32_t status       : 1;
        uint32_t reservedlow  : 1;
        uint32_t messagetype  : 3;
        uint32_t vector       : 8;
    } lvt_t;

    struct smpcomm {
        uint32_t id;            // Accessible via [SMPCOMM_BASE + 0x00].
        uint32_t magic;         // Accessible via [SMPCOMM_BASE + 0x04].
        uint64_t cr3;           // Accessible via [SMPCOMM_BASE + 0x08].
        uint64_t rsp;           // Accessible via [SMPCOMM_BASE + 0x10].
        uint64_t entry;         // Accessible via [SMPCOMM_BASE + 0x18].
        uint16_t gdtsize;       // Accessible via [SMPCOMM_BASE + 0x20].
        uint64_t gdtaddress;    // Accessible via [SMPCOMM_BASE + 0x28].
    } __attribute__((packed));

    struct smpinfo {
        void *address;
        uint64_t codesize;
        void *execute;
        struct smpcomm *shared;
        uint32_t magic;
    } __attribute__((packed));

    struct isolist {
        acpi::madt_entry_iso_t **pointers;
        size_t count;
    };

    struct nmilist {
        acpi::madt_entry_nmi_t **pointers;
        size_t count;
    };

    // Local processor APIC IDs and number of APs.
    extern uint8_t processors[ACPI_MAX_PROCESSORS];
    extern size_t cpucount;

    // ISO and NMI lists and LAPIC/IOAPIC base addresses.
    extern struct isolist isos;
    extern struct nmilist nmis;
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

    // Send an IPI to another local APIC given an ID, type and vector.
    void sendipi(uint8_t dsh, uint8_t id, uint8_t type, uint8_t vector);

    // Enable the current procesor's local APIC.
    void enablelocal(void);

    // Initialise other APs after initialisebsp() has been run.
    void initialiseaps(void);

    // Run by the bootstrap processor to perform initial APIC setup.
    void initialisebsp(void);
}

#endif
