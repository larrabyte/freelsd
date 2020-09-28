#ifndef FREELSD_KERNEL_PIC_HEADER
#define FREELSD_KERNEL_PIC_HEADER

#include <stdint.h>

#define MASTER_PIC_COMMAND  0x20
#define MASTER_PIC_DATA     0x21
#define SLAVE_PIC_COMMAND   0xA0
#define SLAVE_PIC_DATA      0xA1
#define PIC_READ_IRR        0x0A
#define PIC_READ_ISR        0x0B

namespace pic {
    // Enable the Programmable Interrupt Controller.
    void enable(void);

    // Disable the Programmable Interrupt Controller.
    void disable(void);

    // Send an EOI command to the master and slave PICs.
    void sendeoi(uint64_t vector);
}

#endif
