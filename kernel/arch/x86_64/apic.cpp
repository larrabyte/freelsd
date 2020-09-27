#include <interrupts.hpp>
#include <mem/virt.hpp>
#include <mem/phys.hpp>
#include <logger.hpp>
#include <apic.hpp>

extern "C" {
    // Model-specific register read/write functions.
    uint64_t readmsr(uint64_t msr);
    void writemsr(uint64_t msr, uint64_t value);
}

namespace apic {
    apic_config_iso_t isolist;
    apic_config_nmi_t nmilist;
    uintptr_t localbase = 0;
    uintptr_t iobase = 0;

    uint32_t readlocal(localregs_t index) {
        uint32_t volatile *lapicreg = (uint32_t*) (localbase + index);
        return *lapicreg;
    }

    void writelocal(localregs_t index, uint32_t value) {
        uint32_t volatile *lapicreg = (uint32_t*) (localbase + index);
        *lapicreg = value;
    }

    uint64_t readio(uint8_t index, bool read64) {
        uint32_t volatile *datawindow = (uint32_t*) (iobase + 0x10);
        uint32_t volatile *regselect = (uint32_t*) iobase;
        uint64_t regvalue = 0;

        if(read64) {
            // Read the upper 32-bits if read64 is true.
            *regselect = index + 1;
            regvalue = ((uint64_t) *datawindow << 32);
        }

        *regselect = index;
        regvalue = *datawindow;
        return regvalue;
    }

    void writeio(uint8_t index, uint64_t value, bool write64) {
        uint32_t volatile *datawindow = (uint32_t*) (iobase + 0x10);
        uint32_t volatile *regselect = (uint32_t*) iobase;

        if(write64) {
            // Write the upper 32-bits if write64 is true.
            *regselect = index + 1;
            *datawindow = value >> 32;
        }

        *regselect = index;
        *datawindow = value & 0xFFFFFFFF;
    }

    void redirectio(uint8_t irq, uint8_t vector, uint8_t id, io_deliverymode_t mode) {
        uint8_t redirectindex = irq * 0x2 + 0x10;

        // Bit 16 controls interrupt masking, ID is 4 bits.
        uint64_t entry = readio(redirectindex, true);
        entry |= (uint64_t) id << 56 | mode << 8 | vector;
        entry &= ~(1 << 16);

        writeio(redirectindex, entry, true);
    }

    void enablelocal(void) {
        writelocal(LAPIC_SPURIOUS_INTERRUPT_REG, readlocal(LAPIC_SPURIOUS_INTERRUPT_REG) | 0x1FF);
    }

    void initialisebsp(void) {
        // Disable the 8259 PIC before initialising the APIC.
        idt::disablepic();

        // Mark the physical addresses of the LAPIC and IOAPIC as in use.
        localbase = (uintptr_t) readmsr(APIC_BASE_MSR) & 0xFFFFFFFFFFFFF000;
        mem::markphysused(localbase, PGE_PTE_ADDRSPACE);
        mem::markphysused(iobase, PGE_PTE_ADDRSPACE);

        // Find some free MMIO address space to allocate them to.
        localbase = (uintptr_t) mem::allocatemmio(localbase, 1);
        iobase = (uintptr_t) mem::allocatemmio(iobase, 1);
        enablelocal();

        // Use interrupt source override information to redirect IRQs.
        for(size_t i = 0; i < isolist.count; i++) {
            acpi::madt_entry_iso_t *iso = isolist.isos[i];
            redirectio(iso->gsi, iso->sourceirq + 0x20, 0x00, IOAPIC_DELIVERY_FIXED);
        }

        // Redirect the keyboard to GSI 1 (int. vector 0x21).
        redirectio(0x01, 0x21, 0x00, IOAPIC_DELIVERY_FIXED);
        log::info("[osapic] local APIC base address: %p\n", localbase);
        log::info("[osapic] I/O APIC base address: %p\n\n", iobase);
    }
}
