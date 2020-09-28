#include <hwio.hpp>
#include <pic.hpp>

namespace pic {
    void enable(void) {
        outportb(MASTER_PIC_COMMAND, 0x11);
        outportb(SLAVE_PIC_COMMAND, 0x11);
        outportb(MASTER_PIC_DATA, 0x20);
        outportb(SLAVE_PIC_DATA, 0x28);
        outportb(MASTER_PIC_DATA, 0x04);
        outportb(SLAVE_PIC_DATA, 0x02);
        outportb(MASTER_PIC_DATA, 0x01);
        outportb(SLAVE_PIC_DATA, 0x01);
        outportb(MASTER_PIC_DATA, 0x00);
        outportb(SLAVE_PIC_DATA, 0x00);
    }

    void disable(void) {
        outportb(MASTER_PIC_COMMAND, 0x11);
        outportb(SLAVE_PIC_COMMAND, 0x11);
        outportb(MASTER_PIC_DATA, 0xE0);
        outportb(SLAVE_PIC_DATA, 0xE8);
        outportb(MASTER_PIC_DATA, 0x04);
        outportb(SLAVE_PIC_DATA, 0x02);
        outportb(MASTER_PIC_DATA, 0x01);
        outportb(SLAVE_PIC_DATA, 0x01);
        outportb(MASTER_PIC_DATA, 0xFF);
        outportb(SLAVE_PIC_DATA, 0xFF);
    }

    void sendeoi(uint64_t vector) {
        if(vector >= 40) outportb(SLAVE_PIC_COMMAND, 0x20);
        outportb(MASTER_PIC_COMMAND, 0x20);
    }
}
