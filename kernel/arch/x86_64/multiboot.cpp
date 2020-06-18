#include <multiboot.hpp>
#include <serial.hpp>
#include <stddef.h>
#include <stdint.h>

namespace mboot {
    static size_t mbisize;
    static mb_tag_t *cursor;
    mb_info_t info;

    void initialise(uintptr_t mbaddr) {
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
