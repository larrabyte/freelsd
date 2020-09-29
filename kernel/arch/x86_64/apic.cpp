#include <interrupts.hpp>
#include <mem/virt.hpp>
#include <mem/phys.hpp>
#include <logger.hpp>
#include <apic.hpp>
#include <pic.hpp>

extern "C" {
    // Model-specific register read/write functions.
    uint64_t readmsr(uint64_t msr);
    void writemsr(uint64_t msr, uint64_t value);
}

namespace apic {
    uintptr_t localbase = 0;
    uintptr_t iobase = 0;
    struct isolist isos;
    struct nmilist nmis;

    uint32_t readlocal(registers_t index) {
        uint32_t volatile *lapicreg = (uint32_t*) (localbase + index);
        return *lapicreg;
    }

    void writelocal(registers_t index, uint32_t value) {
        uint32_t volatile *lapicreg = (uint32_t*) (localbase + index);
        *lapicreg = value;
    }

    uint64_t readio(registers_t index, bool read64) {
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

    void writeio(registers_t index, uint64_t value, bool write64) {
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

    void enablelocal(void) {
        // Get the APIC ID of the current processor.
        uint32_t apicid = readlocal(LAPIC_IDENTIFIER_REG) >> 24;

        // Configure the local APIC's LINT0 and LINT1 lines.
        for(size_t i = 0; i < nmis.count; i++) {
            acpi::madt_entry_nmi_t *nmi = nmis.pointers[i];
            if(nmi->procid == apicid || nmi->procid == 0xFF) {
                registers_t index = (nmi->lint == 0) ? LAPIC_LINT0_VECTOR_ENTRY_REG : LAPIC_LINT1_VECTOR_ENTRY_REG;
                lvt_t localvector = readlocal(index);
                localvector.messagetype = APIC_MSGTYPE_NMI;
                writelocal(index, localvector);
            }
        }

        // Enable the local APIC through the spurious interrupt register.
        writelocal(LAPIC_SPURIOUS_INTERRUPT_REG, readlocal(LAPIC_SPURIOUS_INTERRUPT_REG) | 0x1FF);
    }

    void initialisebsp(void) {
        // Globally enable the APIC and disable the 8259 PIC before proceeding.
        writemsr(APIC_BASEADDRESS_MSR, readmsr(APIC_BASEADDRESS_MSR) | 1 << 11);
        pic::disable();

        // Mark the physical addresses of the LAPIC and IOAPIC as in use.
        localbase = (uintptr_t) readmsr(APIC_BASEADDRESS_MSR) & 0xFFFFFFFFFFFFF000;
        mem::markphysused(localbase, PGE_PTE_ADDRSPACE);
        mem::markphysused(iobase, PGE_PTE_ADDRSPACE);

        // Find some free MMIO address space to allocate them to.
        localbase = (uintptr_t) mem::allocatemmio(localbase, 1);
        iobase = (uintptr_t) mem::allocatemmio(iobase, 1);
        enablelocal();

        // Assume a 1:1 mapping of I/O APIC lines and ISA IRQs.
        ioredtbl_t basicformat = 0;
        basicformat.masked = 1;

        for(size_t i = 0; i < 24; i++) {
            basicformat.vector = 0x20 + i;
            writeio(ioindex(i), basicformat, true);
        }

        // Use interrupt source override information to make any necessary changes.
        for(size_t i = 0; i < isos.count; i++) {
            acpi::madt_entry_iso_t *iso = isos.pointers[i];
            ioredtbl_t entry = readio(ioindex(iso->gsi), true);

            if((iso->flags & 0x2) == 0x3) entry.polarity = 1;
            if((iso->flags & 0xC) == 0xC) entry.trigger = 1;
            entry.vector = 0x20 + iso->sourceirq;
            writeio(ioindex(iso->gsi), entry, true);
        }

        // Unmask GSI 1 and GSI 2 (mapped to IRQ1 and IRQ0, vectors 0x20 and 0x21).
        writeio(ioindex(1), readio(ioindex(1), true) & ~(APIC_INTERRUPT_MASK), true);
        writeio(ioindex(2), readio(ioindex(2), true) & ~(APIC_INTERRUPT_MASK), true);
        log::info("[osapic] local APIC base address: %p\n", localbase);
        log::info("[osapic] I/O APIC base addresses: %p\n\n", iobase);
    }
}
