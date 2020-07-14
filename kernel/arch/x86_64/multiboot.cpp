#include <multiboot.hpp>
#include <serial.hpp>
#include <stddef.h>
#include <stdint.h>

namespace mboot {
    static size_t mbisize;
    static mb_tag_t *cursor;
    mb_info_t info;

    static const char *mmaptypes[] = {
        "UNKNOWN  ",
        "AVAILABLE",
        "RESERVED ",
        "ACPI REC.",
        "NVS RAM  ",
        "BAD AREA "
    };

    const char *getmmaptype(uint8_t index) {
        return mmaptypes[index];
    }

    void initialise(uint64_t magic, uintptr_t mbaddr) {
        // Make sure that the multiboot magic value is present and the struct address is aligned.
        if(magic != MULTIBOOT2_BOOTLOADER_MAGIC) serial::printf("[kpanic] %p: bootloader non-multiboot2 compliant.\n", magic);
        else if(mbaddr & 0x07) serial::printf("[kpanic] %p: multiboot2 struct unaligned.\n", mbaddr);

        mbisize = *(size_t*) mbaddr;
        cursor = (mb_tag_t*) (mbaddr + 8);

        while(cursor->type != MULTIBOOT_TAG_TYPE_END) {
            switch(cursor->type) {
                case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME: info.bootloader = ((mb_tag_string_t*) cursor)->string; break;
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
