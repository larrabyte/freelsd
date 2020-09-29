#include <interrupts.hpp>
#include <mem/libc.hpp>
#include <mem/virt.hpp>
#include <mem/phys.hpp>
#include <logger.hpp>
#include <errors.hpp>
#include <timer.hpp>
#include <apic.hpp>
#include <pic.hpp>

// Model-specific register read/write functions.
extern "C" uint64_t readmsr(uint64_t msr);
extern "C" void writemsr(uint64_t msr, uint64_t value);

// SMP data from trampoline.asm.
extern "C" apic::smpdata_t smpdata;

namespace apic {
    uint8_t processors[ACPI_MAX_PROCESSORS];
    size_t cpucount = 0;

    struct isolist isos;
    struct nmilist nmis;
    uintptr_t localbase;
    uintptr_t iobase;

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
            *regselect = index + 1;
            *datawindow = value >> 32;
        }

        *regselect = index;
        *datawindow = value & 0xFFFFFFFF;
    }

    void sendipi(uint8_t dsh, uint8_t id, uint8_t type, uint8_t vector) {
        uint32_t hi = (uint32_t) id << 24;
        uint32_t low = dsh << 18 | type << 8 | vector;

        // Writing to the lower 32-bits sends the IPI, write high first.
        writelocal(LAPIC_HIGH_INT_COMMAND_REG, hi);
        writelocal(LAPIC_LOW_INT_COMMAND_REG, low);
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

    void initialiseaps(void) {
        // We already know that addresses 0x1000-0x200000 are mapped.
        uint8_t *smparea = (uint8_t*) mem::allocatephys(2);
        if((uintptr_t) smparea > 0xFF000) panic("could not find space for SMP initialisation!");
        log::trace("[osapic] SMP trampoline area allocated at 0x%lx.\n", smparea);
        log::trace("[osapic] SMP trampoline code size: %zd bytes.\n\n", smpdata.codesize);

        memset(smparea, 0xF4, 0x1000);                       // Cover the SMP code page in HLT instructions.
        memset(smparea + 0x1000, 0x00, 0x1000);              // Zero out the SMP stack page.
        memcpy(smparea, smpdata.address, smpdata.codesize);  // Copy the bytecode from the trampoline the SMP code page.

        sendipi(APIC_DSHTYPE_IGNORE, processors[1], APIC_MSGTYPE_INIT, 0);
        timer::sleep(10); // Wait 10ms for the INIT IPI to set the AP's state as specified in the MPS.
        sendipi(APIC_DSHTYPE_IGNORE, processors[1], APIC_MSGTYPE_STARTUP, (uintptr_t) smparea >> 12);
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
        writeio(ioindex(1), readio(ioindex(1), true) & ~(APIC_INTMASK_BIT), true);
        writeio(ioindex(2), readio(ioindex(2), true) & ~(APIC_INTMASK_BIT), true);
        log::trace("[osapic] local APIC base address: %p\n", localbase);
        log::trace("[osapic] I/O APIC base addresses: %p\n", iobase);
    }
}
