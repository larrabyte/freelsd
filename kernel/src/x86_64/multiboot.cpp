#include <multiboot.hpp>
#include <serial.hpp>

namespace mboot {
    static size_t mbisize;
    static mb_tag_t *cursor;
    mb_info_t info;

    static const char *mmaptypes[] = {
        "UNKNOWN   ",
        "AVAILABLE ",
        "RESERVED  ",
        "ACPI REC. ",
        "NVS MEMORY",
        "BAD MEMORY",
        "PERSISTENT",
        "COREBOOT  ",
        "FIRMWARE  "
    };

    const char *getmmaptype(uint8_t index) {
        // If the index is within standard boundaries, simply return from mmaptypes.
        if(index > 0 && index <= 5) return mmaptypes[index];

        // Non-standard!
        switch(index) {
            case 07: return mmaptypes[6]; break;
            case 12: return mmaptypes[6]; break;
            case 16: return mmaptypes[7]; break;
            case 20: return mmaptypes[8]; break;
            default: return mmaptypes[0]; break;
        }
    }

    void initialise(uint64_t magic, uintptr_t mbaddr) {
        // Make sure that the multiboot magic value is present and the struct address is aligned.
        if(magic != MULTIBOOT2_BOOTLOADER_MAGIC) serial::printf("[kpanic] %p: bootloader non-multiboot2 compliant.\n", magic);
        if(mbaddr & 0x07) serial::printf("[kpanic] %p: multiboot2 struct unaligned.\n", mbaddr);

        mbisize = *(size_t*) mbaddr;
        cursor = (mb_tag_t*) (mbaddr + 8);

        while(cursor->type != MULTIBOOT_TAG_TYPE_END) {
            switch(cursor->type) {
                case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME: info.bootloader = ((mb_tag_string_t*) cursor)->string; break;
                case MULTIBOOT_TAG_TYPE_ACPI_NEW: info.rsdp = (acpi::rsdp_t*) ((mb_tag_acpi_t*) cursor)->rsdp; break;
                case MULTIBOOT_TAG_TYPE_ACPI_OLD: info.rsdp = (acpi::rsdp_t*) ((mb_tag_acpi_t*) cursor)->rsdp; break;
                case MULTIBOOT_TAG_TYPE_CMDLINE: info.cmdline = ((mb_tag_string_t*) cursor)->string; break;
                case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO: info.meminfo = (mb_tag_basicmem_t*) cursor; break;
                case MULTIBOOT_TAG_TYPE_FRAMEBUFFER: info.fbinfo = (mb_tag_framebuffer_t*) cursor; break;
                case MULTIBOOT_TAG_TYPE_BOOTDEV: info.bootdev = (mb_tag_bootdev_t*) cursor; break;
                case MULTIBOOT_TAG_TYPE_MMAP: info.mmap = (mb_tag_mmap_t*) cursor; break;
                default: break;
            }

            cursor = (mb_tag_t*) ((uintptr_t) cursor + ((cursor->size + 0x7) & ~0x7));
        }
    }
}
